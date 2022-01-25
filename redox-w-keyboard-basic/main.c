/*
 * Either COMPILE_RIGHT or COMPILE_LEFT has to be defined from the make call to allow proper functionality
 */
#include <string.h>
#include "redox-w.h"
#include "nrf_drv_config.h"
#include "nrf_gzll.h"
#include "app_uart.h"
#include "app_error.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_rtc.h"
#include "nrf51_bitfields.h"
#include "nrf51.h"
#include "nrf_drv_uart.h"
#include "nrf_delay.h"

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                           /**< UART RX buffer size. */


#define RX_PIN_NUMBER  25
#define TX_PIN_NUMBER  24
#define CTS_PIN_NUMBER 11
#define RTS_PIN_NUMBER 12
#define HWFC           false

// Binary printing
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x40 ? '#' : '.'), \
  (byte & 0x20 ? '#' : '.'), \
  (byte & 0x10 ? '#' : '.'), \
  (byte & 0x08 ? '#' : '.'), \
  (byte & 0x04 ? '#' : '.'), \
  (byte & 0x02 ? '#' : '.'), \
  (byte & 0x01 ? '#' : '.')

// Debug helper variables
extern nrf_gzll_error_code_t nrf_gzll_error_code;   ///< Error code
static FLASH_CTRL flash_t;
static uint8_t keys_view[5];
static bool switch_flag = false;
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

/*****************************************************************************/
/** Configuration */
/*****************************************************************************/

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(1); /**< Declaring an instance of nrf_drv_rtc for RTC1. */


// Data and acknowledgement payloads
static uint8_t ack_payload[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH]; ///< Placeholder for received ACK payloads from Host.

// Debounce time (dependent on tick frequency)
#define DEBOUNCE 5
// Mark as inactive after a number of ticks:
#define INACTIVITY_THRESHOLD 300 // 0.3sec

#ifdef COMPILE_LEFT
static uint8_t channel_table[3]={4, 42, 77};
#endif
#ifdef COMPILE_RIGHT
static uint8_t channel_table[3]={25, 63, 33};
#endif

/** @brief Function for erasing a page in flash.
 *
 * @param page_address Address of the first word in the page to be erased.
 */
static void flash_page_erase(uint32_t * page_address)
{
    // Turn on flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Erase page:
    NRF_NVMC->ERASEPAGE = (uint32_t)page_address;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}


/** @brief Function for filling a page in flash with a value.
 *
 * @param[in] address Address of the first word in the page to be filled.
 * @param[in] value Value to be written to flash.
 */
static void flash_word_write(uint32_t * address, uint32_t value)
{
    // Turn on flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    *address = value;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}


// Setup switch pins with pullups
static void gpio_config(void)
{
    nrf_gpio_cfg_sense_input(R01, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
    nrf_gpio_cfg_sense_input(R02, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
    nrf_gpio_cfg_sense_input(R03, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
    nrf_gpio_cfg_sense_input(R04, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
    nrf_gpio_cfg_sense_input(R05, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);

    nrf_gpio_cfg_output(C01);
    nrf_gpio_cfg_output(C02);
    nrf_gpio_cfg_output(C03);
    nrf_gpio_cfg_output(C04);
    nrf_gpio_cfg_output(C05);
    nrf_gpio_cfg_output(C06);
    nrf_gpio_cfg_output(C07);

    nrf_gpio_pin_clear(C01);
    nrf_gpio_pin_clear(C02);
    nrf_gpio_pin_clear(C03);
    nrf_gpio_pin_clear(C04);
    nrf_gpio_pin_clear(C05);
    nrf_gpio_pin_clear(C06);
    nrf_gpio_pin_clear(C07);
}

// Return the key states
static void read_keys(uint8_t *row_stat)
{
    unsigned short c;
    uint32_t input = 0;
    static const uint32_t COL_PINS[] = { C01, C02, C03, C04, C05, C06, C07 };

    // scan matrix by columns
    for (c = 0; c < COLUMNS; ++c) {
        // Force the compiler to add one cycle gap between activating
        // the column pin and reading the input to allow some time for
        // it to be come stable. Note that compile optimizations are
        // not allowed for next three statements. Setting the pin
        // write a location which is marked as volatile
        // (NRF_GPIO->OUTSET) and reading the input is also a memory
        // access to a location which is marked as volatile too
        // (NRF_GPIO->IN).
        nrf_gpio_pin_set(COL_PINS[c]);
        asm volatile("nop");
        input = NRF_GPIO->IN;
        row_stat[0] = (row_stat[0] << 1) | ((input >> R01) & 1);
        row_stat[1] = (row_stat[1] << 1) | ((input >> R02) & 1);
        row_stat[2] = (row_stat[2] << 1) | ((input >> R03) & 1);
        row_stat[3] = (row_stat[3] << 1) | ((input >> R04) & 1);
        row_stat[4] = (row_stat[4] << 1) | ((input >> R05) & 1);
        nrf_gpio_pin_clear(COL_PINS[c]);
    }

}

static bool compare_keys(const uint8_t* first, const uint8_t* second,
                         uint32_t size)
{
    for(int i=0; i < size; i++)
    {
        if (first[i] != second[i])
        {
          return false;
        }
    }
    return true;
}

static bool empty_keys(const uint8_t* keys_buffer)
{
    for(int i=0; i < ROWS; i++)
    {
        if (keys_buffer[i])
        {
          return false;
        }
    }
    return true;
}

static void pipe_addr_store(uint32_t addr0, uint32_t addr1)
{
    if(flash_t.used_size < flash_t.pg_size)
    {
       // Erase page:
       flash_page_erase(flash_t.page_start_addr);
       // Write to flash
       if (flash_t.patold_0 != addr0)
       {
           flash_t.patold_0 = addr0;
           flash_word_write(flash_t.pipe0_store_addr, (uint32_t)addr0);
           flash_t.used_size += 4;
#ifdef DEBUG
           printf("addr0:%p was write to flash\n\r", (void *)addr0);
#endif
       }
       if (flash_t.patold_1 != addr1)
       {
           flash_t.patold_1 = addr1;
           flash_word_write(flash_t.pipe1_store_addr, (uint32_t)addr1);
           flash_t.used_size += 4;
#ifdef DEBUG
           printf("addr1:%p was write to flash\n\r", (void *)addr1);
#endif
       }
    }
}

static bool check_mask(const uint8_t* keys_buffer)
{
	/* pre press */
	if (!((keys_buffer[ROW1] & MASK_COL0)))
		return false;

	if (keys_buffer[ROW0] & MASK_COL1) {
		pipe_addr_store(BASE_ADDR0, BASE_ADDR1);
#ifdef DEBUG
		printf("ad0\n\r");
#endif
		return true;
	} else if (keys_buffer[ROW0] & MASK_COL2) {
		pipe_addr_store(BASE_ADDR2, BASE_ADDR3);
#ifdef DEBUG
		printf("ad1\n\r");
#endif
		return true;
	} else if (keys_buffer[ROW0] & MASK_COL3) {
		pipe_addr_store(BASE_ADDR4, BASE_ADDR5);
#ifdef DEBUG
		printf("ad2\n\r");
#endif
		return true;
	} else if (keys_buffer[ROW0] & MASK_COL4) {
		pipe_addr_store(BASE_ADDR6, BASE_ADDR7);
#ifdef DEBUG
		printf("ad3\n\r");
#endif
		return true;
	} else if (keys_buffer[ROW0] & MASK_COL5) {
		pipe_addr_store(BASE_ADDR8, BASE_ADDR9);
#ifdef DEBUG
		printf("ad4\n\r");
#endif
		return true;
	} else if (keys_buffer[ROW0] & MASK_COL6) {
		pipe_addr_store(BASE_ADDR10, BASE_ADDR11);
#ifdef DEBUG
		printf("ad5\n\r");
#endif
		return true;
	} else if (keys_buffer[ROW1] & MASK_COL6) {
		pipe_addr_store(BASE_ADDR12, BASE_ADDR13);
#ifdef DEBUG
		printf("ad6\n\r");
#endif
		return true;
	} else if (keys_buffer[ROW2] & MASK_COL6) {
		pipe_addr_store(BASE_ADDR14, BASE_ADDR15);
#ifdef DEBUG
		printf("ad7\n\r");
#endif
		return true;
	} else
		return false;
}

static void handle_inactivity(const uint8_t *keys_buffer)
{
    static uint32_t inactivity_ticks = 0;

    // looking for 500 ticks of no keys pressed, to go back to deep sleep
    if (empty_keys(keys_buffer)) {
        inactivity_ticks++;
        if (inactivity_ticks > INACTIVITY_THRESHOLD) {
            nrf_drv_rtc_disable(&rtc);
            nrf_gpio_pin_set(C01);
            nrf_gpio_pin_set(C02);
            nrf_gpio_pin_set(C03);
            nrf_gpio_pin_set(C04);
            nrf_gpio_pin_set(C05);
            nrf_gpio_pin_set(C06);
            nrf_gpio_pin_set(C07);

            inactivity_ticks = 0;

            NRF_POWER->SYSTEMOFF = 1;
        }
    } else {
        inactivity_ticks = 0;
    }
}

#ifdef DEBUG
static void handle_device_switch(const uint8_t *keys_buffer)
{
	// debugging help, for printing keystates to a serial console
	printf(BYTE_TO_BINARY_PATTERN " " \
	       BYTE_TO_BINARY_PATTERN " " \
	       BYTE_TO_BINARY_PATTERN " " \
	       BYTE_TO_BINARY_PATTERN " " \
	       BYTE_TO_BINARY_PATTERN " " \
	       BYTE_TO_BINARY_PATTERN " " \
	       BYTE_TO_BINARY_PATTERN " " \
	       BYTE_TO_BINARY_PATTERN "\r\n\r\n", \
	       BYTE_TO_BINARY(keys_buffer[0]), \
	       BYTE_TO_BINARY(keys_buffer[1]), \
	       BYTE_TO_BINARY(keys_buffer[2]), \
	       BYTE_TO_BINARY(keys_buffer[3]), \
	       BYTE_TO_BINARY(keys_buffer[4]), \
	       BYTE_TO_BINARY(keys_buffer[5]), \
	       BYTE_TO_BINARY(keys_buffer[6]), \
	       BYTE_TO_BINARY(keys_buffer[7]));

}
#endif

static void handle_send(const uint8_t* keys_buffer)
{
    static uint8_t keys_snapshot[ROWS] = {0};
    static uint32_t debounce_ticks = 0;

    const bool no_change = compare_keys(keys_buffer, keys_snapshot, ROWS);
    if (no_change) {
        debounce_ticks++;
        // debouncing - send only if the keys state has been stable
        // for DEBOUNCE ticks
        if (debounce_ticks == DEBOUNCE) {
            // Assemble packet and send to receiver
            nrf_gzll_add_packet_to_tx_fifo(PIPE_NUMBER, keys_snapshot, ROWS);
            debounce_ticks = 0;
        }
    } else {
        // change detected, start over
        debounce_ticks = 0;
        for (int k = 0; k < ROWS; k++) {
            keys_snapshot[k] = keys_buffer[k];
        }
    }
}

// 1000Hz debounce sampling
static void tick(nrf_drv_rtc_int_type_t int_type)
{
    uint8_t keys_buffer[ROWS] = {0, 0, 0, 0, 0};
    read_keys(keys_buffer);
#ifdef DEBUG
    memcpy(keys_view, keys_buffer, sizeof(keys_buffer)/sizeof(keys_buffer[0]));
#endif
    handle_inactivity(keys_buffer);
    check_mask(keys_buffer);
    handle_send(keys_buffer);
}

// Low frequency clock configuration
static void lfclk_config(void)
{
    nrf_drv_clock_init();

    nrf_drv_clock_lfclk_request(NULL);
}

// RTC peripheral configuration
static void rtc_config(void)
{
    //Initialize RTC instance
    nrf_drv_rtc_init(&rtc, NULL, tick);

    //Enable tick event & interrupt
    nrf_drv_rtc_tick_enable(&rtc, true);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}

int main()
{
    uint32_t err_code;
#ifdef DEBUG
    uint32_t address;
#endif
    uint32_t base_address0;
    uint32_t base_address1;
    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
	  UART_BAUDRATE_BAUDRATE_Baud115200
          //UART_BAUDRATE_BAUDRATE_Baud1M
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);

    /* Read flash to get address*/
    flash_t.patold_0  = 0;
    flash_t.patold_1  = 0;
    flash_t.pg_size = NRF_FICR->CODEPAGESIZE;
    flash_t.pg_num  = NRF_FICR->CODESIZE - 1;  // Use last page in flash

    // calculate store address:
    flash_t.page_start_addr  = (uint32_t *)(flash_t.pg_size * flash_t.pg_num);
    flash_t.pipe0_store_addr = (uint32_t *)flash_t.page_start_addr;
    flash_t.pipe1_store_addr = (uint32_t *)(flash_t.pipe0_store_addr + 4);

    base_address0 = (uint32_t)*(flash_t.pipe0_store_addr);
#ifdef DEBUG
    printf("read-flash-pipe0: %p\n\r\n\r", (void *)base_address0);
#endif
    base_address1 = (uint32_t)*(flash_t.pipe1_store_addr);
#ifdef DEBUG
    printf("read-flash-pipe1: %p\n\r\n\r", (void *)base_address1);
#endif
    if (0 == (base_address1 - base_address0)) {
	base_address0 = BASE_ADDR0;
	base_address1 = BASE_ADDR1;
#ifdef DEBUG
	printf("First boot\n\r");
#endif
    }

    // Initialize Gazell
    nrf_gzll_init(NRF_GZLL_MODE_DEVICE);

    // Attempt sending every packet up to 100 times
    nrf_gzll_set_max_tx_attempts(100);
    nrf_gzll_set_timeslots_per_channel(4);
    nrf_gzll_set_channel_table(channel_table,3);
    nrf_gzll_set_datarate(NRF_GZLL_DATARATE_1MBIT);
    nrf_gzll_set_timeslot_period(900);

    // Set Addressing
    nrf_gzll_set_base_address_0(base_address0);
    nrf_gzll_set_base_address_1(base_address1);
#ifdef DEBUG
    address = nrf_gzll_get_base_address_0();
    printf("get-cur-pipe0: %p\n\r\n\r", (void *)address);
    address = nrf_gzll_get_base_address_1();
    printf("get-cur-pipe1: %p\n\r\n\r", (void *)address);
#endif
    // Enable Gazell to start sending over the air
    nrf_gzll_enable();

    // Configure 32kHz xtal oscillator
    lfclk_config();

    // Configure RTC peripherals with ticks
    rtc_config();

    // Configure all keys as inputs with pullups
    gpio_config();

    // Main loop, constantly sleep, waiting for RTC and gpio IRQs
    while(1)
    {
#ifdef DEBUG
	handle_device_switch(keys_view);
	nrf_delay_us(100000);
#endif
        __SEV();
        __WFE();
        __WFE();
    }
}


/*****************************************************************************/
/** Gazell callback function definitions  */
/*****************************************************************************/

void  nrf_gzll_device_tx_success(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
    uint32_t ack_payload_length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;

    if (tx_info.payload_received_in_ack)
    {
        // Pop packet and write first byte of the payload to the GPIO port.
        nrf_gzll_fetch_packet_from_rx_fifo(pipe, ack_payload, &ack_payload_length);
    }
}

// no action is taken when a packet fails to send, this might need to change
void nrf_gzll_device_tx_failed(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{

}

// Callbacks not needed
void nrf_gzll_host_rx_data_ready(uint32_t pipe, nrf_gzll_host_rx_info_t rx_info)
{}
void nrf_gzll_disabled()
{}
