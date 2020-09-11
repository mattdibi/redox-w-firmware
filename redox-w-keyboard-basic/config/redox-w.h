#ifndef __REDOX_W_FIRMWARE_CONFIG_REDOX_W_H__
#define __REDOX_W_FIRMWARE_CONFIG_REDOX_W_H__

#if defined(COMPILE_LEFT) && defined(COMPILE_RIGHT)
#error "Only one of COMPILE_LEFT and COMPILE_RIGHT can be defined at once."
#endif

#ifdef COMPILE_LEFT

#define PIPE_NUMBER 0

#define C01 10
#define C02 9
#define C03 7
#define C04 6
#define C05 5
#define C06 4
#define C07 3
#define R01 19
#define R02 18
#define R03 17
#define R04 14
#define R05 13

#endif

#ifdef COMPILE_RIGHT

#define PIPE_NUMBER 1

#define C01 6
#define C02 5
#define C03 4
#define C04 3
#define C05 2
#define C06 0
#define C07 30
#define R01 21
#define R02 22
#define R03 23
#define R04 28
#define R05 29

#endif

#define COLUMNS 7
#define ROWS 5

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#endif /* __REDOX_W_FIRMWARE_CONFIG_REDOX_W_H__ */
