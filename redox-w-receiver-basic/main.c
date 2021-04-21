
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
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


int main(void)
{
    uint32_t err_code;
    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud1M
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);

    // Initialize Gazell
    nrf_gzll_init(NRF_GZLL_MODE_HOST);
    nrf_gzll_set_channel_table(channel_table,6);
    nrf_gzll_set_datarate(NRF_GZLL_DATARATE_1MBIT);
    nrf_gzll_set_timeslot_period(900);

    // Addressing
    nrf_gzll_set_base_address_0(0x01020304);
    nrf_gzll_set_base_address_1(0x05060708);

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
