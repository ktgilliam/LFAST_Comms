#define EL_AXIS 0
#define AZ_AXIS 1

#define POS_EL_BTN 2 // up - green
#define NEG_EL_BTN 3 // down - yellow
#define NEG_AZ_BTN 4 // left - orange
#define POS_AZ_BTN 5 // right - red

#define BIG_BUTTON_A 5
#define BIG_BUTTON_B 6

#define SPEED_POT_PIN A9

#define SERIAL_CH Serial
#define SERIAL_BAUD 9600

#define STILL_ALIVE_INTERVAL 1000000
#define SPEED_POT_SCALE (1.0/1023.0)
#define BOUNCE_INTERVAL_MS 50