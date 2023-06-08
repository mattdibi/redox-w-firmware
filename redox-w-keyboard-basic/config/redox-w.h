#ifndef __REDOX_W_FIRMWARE_CONFIG_REDOX_W_H__
#define __REDOX_W_FIRMWARE_CONFIG_REDOX_W_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#if defined(COMPILE_LEFT) && defined(COMPILE_RIGHT)
#error "Only one of COMPILE_LEFT and COMPILE_RIGHT can be defined at once."
#endif

/* TODO: apply this MACRO in main.c */
//#define DEBUG

typedef struct {
    uint32_t * page_start_addr;
    uint32_t * pipe0_store_addr;
    uint32_t * pipe1_store_addr;
    uint8_t    patwr;
    uint8_t    patold_0;
    uint8_t    patold_1;
    uint32_t   used_size;
    uint32_t   pg_size;
    uint32_t   pg_num;
} FLASH_CTRL;

/* base_address */
#define BASE_ADDR0	0x01020304
#define BASE_ADDR1	0x05060708
#define BASE_ADDR2	0x02020202
#define BASE_ADDR3	0x03030303
#define BASE_ADDR4	0x04040404
#define BASE_ADDR5	0x05050505
#define BASE_ADDR6	0x06060606
#define BASE_ADDR7	0x07070707
#define BASE_ADDR8	0x08080808
#define BASE_ADDR9	0x09090909
#define BASE_ADDR10	0x0A0A0A0A
#define BASE_ADDR11	0x0B0B0B0B
#define BASE_ADDR12	0x0C0C0C0C
#define BASE_ADDR13	0x0D0D0D0D
#define BASE_ADDR14	0x0E0E0E0E
#define BASE_ADDR15	0x0F0F0F0F

#ifdef COMPILE_LEFT

#define PIPE_NUMBER 0

#define C01 3
#define C02 4
#define C03 5
#define C04 6
#define C05 7
#define C06 9
#define C07 10

#define R01 19
#define R02 18
#define R03 17
#define R04 14
#define R05 13

#endif

#ifdef COMPILE_RIGHT

#define PIPE_NUMBER 1

#define C01 30
#define C02 0
#define C03 2
#define C04 3
#define C05 4
#define C06 5
#define C07 6

#define R01 21
#define R02 22
#define R03 23
#define R04 28
#define R05 29

#endif

#define COLUMNS 7
#define ROWS 5


#ifdef COMPILE_RIGHT

#define MASK_COL0	(1 << 6)
#define MASK_COL1	(1 << 5)
#define MASK_COL2	(1 << 4)
#define MASK_COL3	(1 << 3)
#define MASK_COL4	(1 << 2)
#define MASK_COL5	(1 << 1)
#define MASK_COL6	(1 << 0)

#endif


#ifdef COMPILE_LEFT

#define MASK_COL0	(1 << 0)
#define MASK_COL1	(1 << 1)
#define MASK_COL2	(1 << 2)
#define MASK_COL3	(1 << 3)
#define MASK_COL4	(1 << 4)
#define MASK_COL5	(1 << 5)
#define MASK_COL6	(1 << 6)

#endif

enum ROWS_SPECIFIED
{
	ROW0,
	ROW1,
	ROW2,
	ROW3,
	ROW4,
};


// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#endif /* __REDOX_W_FIRMWARE_CONFIG_REDOX_W_H__ */
