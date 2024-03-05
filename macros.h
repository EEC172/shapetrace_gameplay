#ifndef MACROS_H
#define MACROS_H

#define MAX_URI_SIZE 128
#define URI_SIZE MAX_URI_SIZE + 1


#define APPLICATION_NAME        "SSL"
#define APPLICATION_VERSION     "1.1.1.EEC.Spring2018"
#define SERVER_NAME             "avjw5sumnrptc-ats.iot.us-east-1.amazonaws.com"
#define GOOGLE_DST_PORT         8443

#define SL_SSL_CA_CERT "/cert/rootCA.der" //starfield class2 rootca (from firefox) // <-- this one works
#define SL_SSL_PRIVATE "/cert/private.der"
#define SL_SSL_CLIENT  "/cert/client.der"


//NEED TO UPDATE THIS FOR IT TO WORK!
#define DATE                25    /* Current Date */
#define MONTH               2     /* Month 1-12 */
#define YEAR                2024  /* Current year */
#define HOUR                23    /* Time - hours */
#define MINUTE              06    /* Time - minutes */
#define SECOND              0     /* Time - seconds */

#define POSTHEADER "POST /things/gb_mr_CC3200/shadow HTTP/1.1\r\n"
#define GETHEADER "GET /things/gb_mr_CC3200/shadow HTTP/1.1\r\n"
#define HOSTHEADER "Host: avjw5sumnrptc-ats.iot.us-east-1.amazonaws.com\r\n"
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"

#define DATA1 "{\"state\": {\r\n\"desired\" : {\r\n\"var\" : \"This message is to our email!\"\r\n}}}\r\n\r\n"

// some helpful macros for systick

// the cc3200's fixed clock frequency of 80 MHz
// note the use of ULL to indicate an unsigned long long constant
#define SYSCLKFREQ 80000000ULL

// macro to convert ticks to microseconds
#define TICKS_TO_US(ticks) \
    ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
    ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))\

// macro to convert microseconds to ticks
#define US_TO_TICKS(us) ((SYSCLKFREQ / 1000000ULL) * (us))

// systick reload value set to 40ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL

#define START_ADDRESS "20001000000010000"

#define ARRAY_0 "1001000001101111"
#define ARRAY_1 "0000000011111111"
#define ARRAY_2 "1000000001111111"
#define ARRAY_3 "0100000010111111"
#define ARRAY_4 "1100000000111111"
#define ARRAY_5 "0010000011011111"
#define ARRAY_6 "1010000001011111"
#define ARRAY_7 "0110000010011111"
#define ARRAY_8 "1110000000011111"
#define ARRAY_9 "0001000011101111"
#define ARRAY_LAST "1011100001000111"
#define ARRAY_MUTE "0010100011010111"

#define INITIAL 0
#define ADDRESS_PROCESSING 1
#define DATA_PROCESSING 17
volatile int state;

#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

#define HORIZONTAL      0
#define VERTICAL        1

#define SPI_IF_BIT_RATE  100000
#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}
#define HORIZONTAL      0
#define VERTICAL        1

#define SLAVE_ADDRESS 0x18
#define RD_LEN        6

#endif //includes.h
