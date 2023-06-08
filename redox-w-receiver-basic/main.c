
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "redox-w.h"
#include "nrf_gpio.h"
#include "app_uart.h"
#include "nrf_drv_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "nrf_gzll.h"

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                           /**< UART RX buffer size. */


#define RX_PIN_NUMBER  25
#define TX_PIN_NUMBER  24
#define CTS_PIN_NUMBER 23
#define RTS_PIN_NUMBER 22
#define HWFC           false


// Define payload length
#define PAYLOAD_LENGTH 5 ///< 5 byte payload length

#define MATRIX_ROWS 10

// ticks for inactive keyboard
// Binary printing
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '#' : '.'), \
  (byte & 0x40 ? '#' : '.'), \
  (byte & 0x20 ? '#' : '.'), \
  (byte & 0x10 ? '#' : '.'), \
  (byte & 0x08 ? '#' : '.'), \
  (byte & 0x04 ? '#' : '.'), \
  (byte & 0x02 ? '#' : '.'), \
  (byte & 0x01 ? '#' : '.')


// Debug helper variables
extern nrf_gzll_error_code_t nrf_gzll_error_code;   ///< Error code
static bool init_ok, enable_ok, push_ok, pop_ok;

static uint8_t channel_table[6]={4, 25, 42, 63, 77, 33};

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

static void keys_init(void)
{
	nrf_gpio_cfg_sense_input(KEY00, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(KEY01, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(KEY02, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(KEY03, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(KEY04, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(KEY05, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(KEY06, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
	nrf_gpio_cfg_sense_input(KEY07, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);

}

static uint8_t bootcheck(void)
{
	uint32_t input = 0;
	uint8_t keys_stat = 0;
	static const uint32_t KEY_PINS[] = {
		KEY00, KEY01, KEY02, KEY03, KEY04, KEY05, KEY06, KEY07 };

        asm volatile("nop");
        asm volatile("nop");
        input = NRF_GPIO->IN;

	keys_stat = ((keys_stat) << 0) | ((input >> KEY_PINS[0]) & 1);

	for(uint8_t i = 1; i < (sizeof(KEY_PINS) / sizeof(KEY_PINS[0])); i++)
	    keys_stat = ((keys_stat) << 1) | ((input >> KEY_PINS[i]) & 1);

	return keys_stat;
}

static void addr_set_from_key_status(uint8_t keys_stat)
{
	switch(keys_stat) {
	case STAT0  :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR0);
		nrf_gzll_set_base_address_1(BASE_ADDR1);
	break;
	case STAT1  :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR2);
		nrf_gzll_set_base_address_1(BASE_ADDR3);
	break;
	case STAT2  :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR4);
		nrf_gzll_set_base_address_1(BASE_ADDR5);
	break;
	case STAT3  :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR6);
		nrf_gzll_set_base_address_1(BASE_ADDR7);
	break;
	case STAT4  :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR8);
		nrf_gzll_set_base_address_1(BASE_ADDR9);
	break;
	case STAT5  :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR10);
		nrf_gzll_set_base_address_1(BASE_ADDR11);
	break;
	case STAT6  :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR12);
		nrf_gzll_set_base_address_1(BASE_ADDR13);
	break;
	case STAT7  :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR14);
		nrf_gzll_set_base_address_1(BASE_ADDR15);
	break;
	default :
		// Addressing
		nrf_gzll_set_base_address_0(BASE_ADDR0);
		nrf_gzll_set_base_address_1(BASE_ADDR1);
	}

}

int main(void)
{
    uint32_t err_code;
    static uint8_t keys_status;
    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud1M
          //UART_BAUDRATE_BAUDRATE_Baud115200
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);

    keys_init();

    // Initialize Gazell
    nrf_gzll_init(NRF_GZLL_MODE_HOST);
    nrf_gzll_set_channel_table(channel_table,6);
    nrf_gzll_set_datarate(NRF_GZLL_DATARATE_1MBIT);
    nrf_gzll_set_timeslot_period(900);

    // Addressing
    keys_status = bootcheck();
    addr_set_from_key_status(keys_status);

    // Enable Gazell to start sending over the air
    nrf_gzll_enable();

    uint8_t matrix[MATRIX_ROWS] = {0};

    // main loop
    while (true)
    {
        for (int pipe = 0; pipe < 2; pipe++) {
            if (!nrf_gzll_get_rx_fifo_packet_count(pipe)) {
                continue;
            }

            uint32_t payload_len = PAYLOAD_LENGTH;
            uint8_t payload[PAYLOAD_LENGTH];
            bool ret = nrf_gzll_fetch_packet_from_rx_fifo(pipe, payload, &payload_len);
            if (ret && payload_len == PAYLOAD_LENGTH) {
                for (int i = 0; i < PAYLOAD_LENGTH; i++) {
                    matrix[i * 2 + pipe] = payload[i];
                }
            }
        }

        // checking for a poll request from QMK
        uint8_t c;
        if (app_uart_get(&c) == NRF_SUCCESS && c == 's')
        {
            // sending data to QMK, and an end byte
            nrf_drv_uart_tx(matrix, 10);
            app_uart_put(0xE0);

            // debugging help, for printing keystates to a serial console
            /*
            for (uint8_t i = 0; i < 10; i++)
            {
                app_uart_put(data_buffer[i]);
            }
            printf(BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN "\r\n", \
                   BYTE_TO_BINARY(data_payload_left[0]), \
                   BYTE_TO_BINARY(data_payload_left[1]), \
                   BYTE_TO_BINARY(data_payload_left[2]), \
                   BYTE_TO_BINARY(data_payload_left[3]), \
                   BYTE_TO_BINARY(data_payload_left[4]));
            nrf_delay_us(100);
            */
        }
        // allowing UART buffers to clear
        nrf_delay_us(10);
    }
}


// Callbacks not needed in this example.
void nrf_gzll_device_tx_success(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info) {}
void nrf_gzll_device_tx_failed(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info) {}
void nrf_gzll_disabled() {}
void nrf_gzll_host_rx_data_ready(uint32_t pipe, nrf_gzll_host_rx_info_t rx_info) {}
