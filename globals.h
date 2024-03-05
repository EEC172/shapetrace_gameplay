#ifndef GLOBALS_H
#define GLOBALS_H

// Include necessary headers here

// Application specific status/error codes
typedef enum {
    // Error codes
    LAN_CONNECTION_FAILED = -0x7D0,
    INTERNET_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
    DEVICE_NOT_IN_STATION_MODE = INTERNET_CONNECTION_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
} e_AppStatusCodes;

typedef struct {
    /* time */
    unsigned long tm_sec;
    unsigned long tm_min;
    unsigned long tm_hour;
    /* date */
    unsigned long tm_day;
    unsigned long tm_mon;
    unsigned long tm_year;
    unsigned long tm_week_day; //not required
    unsigned long tm_year_day; //not required
    unsigned long reserved[3];
} SlDateTime;

// Global variables
extern volatile uint64_t delta;
extern volatile double delta_ms;
extern volatile int systick_cnt;
extern volatile unsigned long IR_intcount;
extern volatile unsigned char IR_intflag;
extern uint64_t systick_get[33];
extern double systick_get_ms[33];
extern char start_and_address[17];
extern char data[16];
extern int letter_count;
extern char text[64];
extern int pressed_button;
extern int prev;
extern int same_button_counter;
extern time_t prev_button_pressed_time;
extern time_t uart_handler_time;
extern double interval;
extern int globalX;
extern int globalY;
extern char dad[64];
extern unsigned char ucCharBuffer[64];
extern uint16_t ui16CharCounter;
extern int uart_int_count;
extern int UART_RX_intflag;

//extern unsigned char reg_offset;
//extern static unsigned long __Errorlog;

#endif // GLOBALS_H
