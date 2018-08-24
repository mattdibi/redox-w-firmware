
#define HAND_SENSE 12
#define RIGHT_HAND false
#define LEFT_HAND true

#define ALPHA_SENSE 20
#define ALPABETICAL false

// left hand pins

#define L_C01 6
#define L_C02 5
#define L_C03 4
#define L_C04 3
#define L_C05 2
#define L_C06 0
#define L_C07 30
#define L_R01 28
#define L_R02 23
#define L_R03 22
#define L_R04 21
#define L_R05 29

// right hand pins

#define R_C01 10
#define R_C02 9
#define R_C03 7
#define R_C04 6
#define R_C05 5
#define R_C06 4
#define R_C07 3
#define R_R01 14
#define R_R02 17
#define R_R03 18
#define R_R04 19
#define R_R05 13

#ifdef COMPILE_LEFT

#define PIPE_NUMBER 0

#define C01 L_C01
#define C02 L_C02
#define C03 L_C03
#define C04 L_C04
#define C05 L_C05
#define C06 L_C06
#define C07 L_C07
#define R01 L_R01
#define R02 L_R02
#define R03 L_R03
#define R04 L_R04
#define R05 L_R05

#endif

#ifdef COMPILE_RIGHT

#define PIPE_NUMBER 1

#define C01 R_C01
#define C02 R_C02
#define C03 R_C03
#define C04 R_C04
#define C05 R_C05
#define C06 R_C06
#define C07 R_C07
#define R01 R_R01
#define R02 R_R02
#define R03 R_R03
#define R04 R_R04
#define R05 R_R05

#endif

#define COLUMNS 7
#define ROWS 5

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}
