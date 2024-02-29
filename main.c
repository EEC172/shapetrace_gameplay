//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************


//*****************************************************************************
//
// Application Name     -   SSL Demo
// Application Overview -   This is a sample application demonstrating the
//                          use of secure sockets on a CC3200 device.The
//                          application connects to an AP and
//                          tries to establish a secure connection to the
//                          Google server.
// Application Details  -
// docs\examples\CC32xx_SSL_Demo_Application.pdf
// or
// http://processors.wiki.ti.com/index.php/CC32xx_SSL_Demo_Application
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup ssl
//! @{
//
//*****************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "spi.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "glcdfont.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "systick.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "uart.h"

//Common interface includes
//#include "pinmux.h"
#include "gpio_if.h"
#include "common.h"
#include "uart_if.h"

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
// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    LAN_CONNECTION_FAILED = -0x7D0,
    INTERNET_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
    DEVICE_NOT_IN_STATION_MODE = INTERNET_CONNECTION_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

typedef struct
{
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
}SlDateTime;


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulPingPacketsRecv = 0; //Number of Ping Packets received
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
signed char    *g_Host = SERVER_NAME;
SlDateTime g_time;
#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

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

volatile uint64_t delta; volatile double delta_ms;

// track systick counter periods elapsed
// if it is not 0, we know the transmission ended
volatile int systick_cnt = 0;

extern void (* const g_pfnVectors[])(void);

volatile unsigned long IR_intcount;
volatile unsigned char IR_intflag;


uint64_t systick_get[33];
double systick_get_ms[33];
char start_and_address[17]; char data[16];
int letter_count = 0;
char text[64];
int pressed_button = 0; int prev = -1; int same_button_counter = 0;
time_t prev_button_pressed_time;
time_t uart_handler_time;
double interval;
int globalX = 0; int globalY = 70;
char dad[64];


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

#define BOARD_TO_BOARD         UARTA1_BASE
#define BOARD_TO_BOARD_PERIPH  PRCM_UARTA1
unsigned char ucCharBuffer[64];
uint16_t ui16CharCounter = 0;
int uart_int_count = 0;
int UART_RX_intflag = 0;

static unsigned long __Errorlog;
//*****************************************************************************
//                 GLOBAL VARIABLES -- End: df
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static long WlanConnect();
static int set_time();
static void BoardInit(void);
static long InitializeAppVariables();
static int tls_connect();
static int connectToAccessPoint();
static int http_post(int, char*);

//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************


//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {
    if(!pWlanEvent) {
        return;
    }

    switch(pWlanEvent->Event) {
        case SL_WLAN_CONNECT_EVENT: {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);

            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'.
            // Applications can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s , "
                       "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                       g_ucConnectionSSID,g_ucConnectionBSSID[0],
                       g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                       g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                       g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT: {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_USER_INITIATED_DISCONNECTION
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code) {
                UART_PRINT("[WLAN EVENT]Device disconnected from the AP: %s,"
                    "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else {
                UART_PRINT("[WLAN ERROR]Device disconnected from the AP AP: %s, "
                           "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
        }
        break;

        default: {
            UART_PRINT("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
        }
        break;
    }
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {
    if(!pNetAppEvent) {
        return;
    }

    switch(pNetAppEvent->Event) {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT: {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            UART_PRINT("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                       "Gateway=%d.%d.%d.%d\n\r",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,0));
        }
        break;

        default: {
            UART_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
        }
        break;
    }
}


//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse) {
    // Unused in this application
}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent) {
    if(!pDevEvent) {
        return;
    }

    //
    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    //
    UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->EventData.deviceEvent.status,
               pDevEvent->EventData.deviceEvent.sender);
}


//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock) {
    if(!pSock) {
        return;
    }

    switch( pSock->Event ) {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->socketAsyncEvent.SockTxFailData.status) {
                case SL_ECLOSE: 
                    UART_PRINT("[SOCK ERROR] - close socket (%d) operation "
                                "failed to transmit all queued packets\n\n", 
                                    pSock->socketAsyncEvent.SockTxFailData.sd);
                    break;
                default: 
                    UART_PRINT("[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                                "(%d) \n\n",
                                pSock->socketAsyncEvent.SockTxFailData.sd, pSock->socketAsyncEvent.SockTxFailData.status);
                  break;
            }
            break;

        default:
            UART_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
          break;
    }
}


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End breadcrumb: s18_df
//*****************************************************************************


//*****************************************************************************
//
//! \brief This function initializes the application variables
//!
//! \param    0 on success else error code
//!
//! \return None
//!
//*****************************************************************************
static long InitializeAppVariables() {
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    g_Host = SERVER_NAME;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
    return SUCCESS;
}


//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to STATION
//!           - Configures connection policy to Auto and AutoSmartConfig
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
static long ConfigureSimpleLinkToDefaultState() {
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode 
    if (ROLE_STA != lMode) {
        if (ROLE_AP == lMode) {
            // If the device is in AP mode, we need to wait for this event 
            // before doing anything 
            while(!IS_IP_ACQUIRED(g_ulStatus)) {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
#endif
            }
        }

        // Switch to STA role and restart 
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again 
        if (ROLE_STA != lRetVal) {
            // We don't want to proceed if the device is not coming up in STA-mode 
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }
    
    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);
    
    UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig 
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, 
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);

    

    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore 
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal) {
        // Wait
        while(IS_CONNECTED(g_ulStatus)) {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, 
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeAppVariables();
    
    return lRetVal; // Success
}


//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void) {
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}


//****************************************************************************
//
//! \brief Connecting to a WLAN Accesspoint
//!
//!  This function connects to the required AP (SSID_NAME) with Security
//!  parameters specified in te form of macros at the top of this file
//!
//! \param  None
//!
//! \return  0 on success else error code
//!
//! \warning    If the WLAN connection fails or we don't aquire an IP
//!            address, It will be stuck in this function forever.
//
//****************************************************************************
static long WlanConnect() {
    SlSecParams_t secParams = {0};
    long lRetVal = 0;

    secParams.Key = SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;

    UART_PRINT("Attempting connection to access point: ");
    UART_PRINT(SSID_NAME);
    UART_PRINT("... ...");
    lRetVal = sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(lRetVal);

    UART_PRINT(" Connected!!!\n\r");


    // Wait for WLAN Event
    while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus))) {
        // Toggle LEDs to Indicate Connection Progress
        _SlNonOsMainLoopTask();
        GPIO_IF_LedOff(MCU_IP_ALLOC_IND);
        MAP_UtilsDelay(800000);
        _SlNonOsMainLoopTask();
        GPIO_IF_LedOn(MCU_IP_ALLOC_IND);
        MAP_UtilsDelay(800000);
    }

    return SUCCESS;

}




long printErrConvenience(char * msg, long retVal) {
    UART_PRINT(msg);
    GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    return retVal;
}


//*****************************************************************************
//
//! This function updates the date and time of CC3200.
//!
//! \param None
//!
//! \return
//!     0 for success, negative otherwise
//!
//*****************************************************************************

static int set_time() {
    long retVal;

    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;

    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                          SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                          sizeof(SlDateTime),(unsigned char *)(&g_time));

    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}

//*****************************************************************************
//
//! This function demonstrates how certificate can be used with SSL.
//! The procedure includes the following steps:
//! 1) connect to an open AP
//! 2) get the server name via a DNS request
//! 3) define all socket options and point to the CA certificate
//! 4) connect to the server via TCP
//!
//! \param None
//!
//! \return  0 on success else error code
//! \return  LED1 is turned solid in case of success
//!    LED2 is turned solid in case of failure
//!
//*****************************************************************************
static int tls_connect() {
    SlSockAddrIn_t    Addr;
    int    iAddrSize;
    unsigned char    ucMethod = SL_SO_SEC_METHOD_TLSV1_2;
    unsigned int uiIP;
//    unsigned int uiCipher = SL_SEC_MASK_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA;
    unsigned int uiCipher = SL_SEC_MASK_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256;
// SL_SEC_MASK_SSL_RSA_WITH_RC4_128_SHA
// SL_SEC_MASK_SSL_RSA_WITH_RC4_128_MD5
// SL_SEC_MASK_TLS_RSA_WITH_AES_256_CBC_SHA
// SL_SEC_MASK_TLS_DHE_RSA_WITH_AES_256_CBC_SHA
// SL_SEC_MASK_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
// SL_SEC_MASK_TLS_ECDHE_RSA_WITH_RC4_128_SHA
// SL_SEC_MASK_TLS_RSA_WITH_AES_128_CBC_SHA256
// SL_SEC_MASK_TLS_RSA_WITH_AES_256_CBC_SHA256
// SL_SEC_MASK_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
// SL_SEC_MASK_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 // does not work (-340, handshake fails)
    long lRetVal = -1;
    int iSockID;

    lRetVal = sl_NetAppDnsGetHostByName(g_Host, strlen((const char *)g_Host),
                                    (unsigned long*)&uiIP, SL_AF_INET);

    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't retrieve the host name \n\r", lRetVal);
    }

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons(GOOGLE_DST_PORT);
    Addr.sin_addr.s_addr = sl_Htonl(uiIP);
    iAddrSize = sizeof(SlSockAddrIn_t);
    //
    // opens a secure socket
    //
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, SL_SEC_SOCKET);
    if( iSockID < 0 ) {
        return printErrConvenience("Device unable to create secure socket \n\r", lRetVal);
    }

    //
    // configure the socket as TLS1.2
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_SECMETHOD, &ucMethod,\
                               sizeof(ucMethod));
    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }
    //
    //configure the socket as ECDHE RSA WITH AES256 CBC SHA
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_SECURE_MASK, &uiCipher,\
                           sizeof(uiCipher));
    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }



/////////////////////////////////
// START: COMMENT THIS OUT IF DISABLING SERVER VERIFICATION
    //
    //configure the socket with CA certificate - for server verification
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, \
                           SL_SO_SECURE_FILES_CA_FILE_NAME, \
                           SL_SSL_CA_CERT, \
                           strlen(SL_SSL_CA_CERT));

    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }
// END: COMMENT THIS OUT IF DISABLING SERVER VERIFICATION
/////////////////////////////////


    //configure the socket with Client Certificate - for server verification
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, \
                SL_SO_SECURE_FILES_CERTIFICATE_FILE_NAME, \
                                    SL_SSL_CLIENT, \
                           strlen(SL_SSL_CLIENT));

    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }

    //configure the socket with Private Key - for server verification
    //
    lRetVal = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, \
            SL_SO_SECURE_FILES_PRIVATE_KEY_FILE_NAME, \
            SL_SSL_PRIVATE, \
                           strlen(SL_SSL_PRIVATE));

    if(lRetVal < 0) {
        return printErrConvenience("Device couldn't set socket options \n\r", lRetVal);
    }


    /* connect to the peer device - Google server */
    lRetVal = sl_Connect(iSockID, ( SlSockAddr_t *)&Addr, iAddrSize);

    if(lRetVal >= 0) {
        UART_PRINT("Device has connected to the website:");
        UART_PRINT(SERVER_NAME);
        UART_PRINT("\n\r");
    }
    else if(lRetVal == SL_ESECSNOVERIFY) {
        UART_PRINT("Device has connected to the website (UNVERIFIED):");
        UART_PRINT(SERVER_NAME);
        UART_PRINT("\n\r");
    }
    else if(lRetVal < 0) {
        UART_PRINT("Device couldn't connect to server:");
        UART_PRINT(SERVER_NAME);
        UART_PRINT("\n\r");
        return printErrConvenience("Device couldn't connect to server \n\r", lRetVal);
    }

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    return iSockID;
}



int connectToAccessPoint() {
    long lRetVal = -1;
    GPIO_IF_LedConfigure(LED1|LED3);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    lRetVal = InitializeAppVariables();
    ASSERT_ON_ERROR(lRetVal);

    //
    // Following function configure the device to default state by cleaning
    // the persistent settings stored in NVMEM (viz. connection profiles &
    // policies, power policy etc)
    //
    // Applications may choose to skip this step if the developer is sure
    // that the device is in its default state at start of applicaton
    //
    // Note that all profiles and persistent settings that were done on the
    // device will be lost
    //
    lRetVal = ConfigureSimpleLinkToDefaultState();
    if(lRetVal < 0) {
      if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
          UART_PRINT("Failed to configure the device in its default state \n\r");

      return lRetVal;
    }

    UART_PRINT("Device is configured in default state \n\r");

    CLR_STATUS_BIT_ALL(g_ulStatus);

    ///
    // Assumption is that the device is configured in station mode already
    // and it is in its default state
    //
    UART_PRINT("Opening sl_start\n\r");
    lRetVal = sl_Start(0, 0, 0);
    if (lRetVal < 0 || ROLE_STA != lRetVal) {
        UART_PRINT("Failed to start the device \n\r");
        return lRetVal;
    }

    UART_PRINT("Device started as STATION \n\r");

    //
    //Connecting to WLAN AP
    //
    lRetVal = WlanConnect();
    if(lRetVal < 0) {
        UART_PRINT("Failed to establish connection w/ an AP \n\r");
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }

    UART_PRINT("Connection established w/ AP and IP is aquired \n\r");
    return 0;
}

/*****************************************************************************/
/**
 * Reset SysTick Counter
 */
static inline void SysTickReset(void) {
    // any write to the ST_CURRENT register clears it
    // after clearing it automatically gets reset without
    // triggering exception logic
    // see reference manual section 3.2.1
    HWREG(NVIC_ST_CURRENT) = 1;

    // clear the global count variable
    systick_cnt = 0;

    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);
}

/**
 * SysTick Interrupt Handler
 *
 * Keep track of whether the systick counter wrapped
 */
static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt++;
}

void ClearArrays(void) {
    if (IR_intcount >= 33) {
        memset(systick_get, 0, sizeof(systick_get));
        memset(systick_get_ms, 0, sizeof(systick_get_ms));
        memset(start_and_address, 0, sizeof(start_and_address));
        memset(data, 0, sizeof(data));
    }
}

bool ConfirmStartAndAddress() { return (strcmp(start_and_address, START_ADDRESS) == 0); }

//IR_intflag = 0;
void InitialState() {
    IR_intcount = 0; IR_intflag = 0;
    if ((delta >= 690000) && (delta <= 730000)) { //range of values for approx 9 ms
        systick_get[IR_intcount] = delta;
        systick_get_ms[IR_intcount] = delta_ms;
        start_and_address[IR_intcount] = '2';
        state = ADDRESS_PROCESSING;
    }
 }

void AddressProcessingState() {
    IR_intcount++;
    //address reading
    if ((delta <= 93000) && (delta >= 80000)) {
        systick_get[IR_intcount] = delta;
        systick_get_ms[IR_intcount] = delta_ms;
        start_and_address[IR_intcount] = '0';
    } else if ((delta >= 165000) && (delta <= 195000)) {
        systick_get[IR_intcount] = delta;
        systick_get_ms[IR_intcount] = delta_ms;
        start_and_address[IR_intcount] = '1';
    } else {
        IR_intcount = 0; ClearArrays(); state = INITIAL;
        return;
    }
    //end of address reading. State will change at IR_intcount = 16
    if (IR_intcount == 16) {
        //mismatch addresses
        if (!ConfirmStartAndAddress()) {
            IR_intcount = 0; ClearArrays(); state = INITIAL;
            return;
        } else {
            state = DATA_PROCESSING;
            return;
        }
    }
}

void DataProcessingState() {
    IR_intcount++;
    //data reading
    if ((delta <= 93000) && (delta >= 80000)) {
        systick_get[IR_intcount] = delta;
        systick_get_ms[IR_intcount] = delta_ms;
        data[IR_intcount-17] = '0';
    } else if ((delta >= 165000) && (delta <= 195000)) {
        systick_get[IR_intcount] = delta;
        systick_get_ms[IR_intcount] = delta_ms;
        data[IR_intcount-17] = '1';
    } else {
        IR_intcount = 0; ClearArrays(); state = INITIAL;
        return;
    }
    //end of data reading
    if (IR_intcount == 32) {
        IR_intcount = 0; IR_intflag = 1; state = INITIAL;
        return;
    }
}

static void GPIOA3IntHandler(void) { // IR handler
    unsigned long ulStatus;
    ulStatus = MAP_GPIOIntStatus (GPIOA3_BASE, true);
    MAP_GPIOIntClear(GPIOA3_BASE, ulStatus);        // clear interrupts on GPIOA3
    delta = SYSTICK_RELOAD_VAL - SysTickValueGet();
    delta_ms = (double)TICKS_TO_US(delta)/1000;
    switch (state) {
            case INITIAL:
                InitialState();
                break;
            case ADDRESS_PROCESSING:
                AddressProcessingState();
                break;
            case DATA_PROCESSING:
                DataProcessingState();
                break;
            default:
                break;
    }
    SysTickReset();
}

void MasterMain()
{

    //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

}

/**
 * Initializes SysTick Module
 */
static void SysTickInit(void) {

    // configure the reset value for the systick countdown register
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);

    // register interrupts on the systick module
    MAP_SysTickIntRegister(SysTickHandler);

    // enable interrupts on systick
    // (trigger SysTickHandler when countdown reaches 0)
    MAP_SysTickIntEnable();

    // enable the systick module itself
    MAP_SysTickEnable();
}

void printText(int type_array) {
    int pixelX = 0;
    int pixelY = 0;
    int i = 0;
    // don't let x or y go past 128
    if (type_array) {
        Report("PRINT FUNC ui16 Char Count: %d\n\r", ui16CharCounter);
        Report("FINAL PRINT FUNC Received Text message: %.*s\n\r", ui16CharCounter, ucCharBuffer);

        for (i = 0; i < ui16CharCounter; i++) {
//            Report('i=%d , char= %c\n\r', i, ucCharBuffer[i]);
            drawChar(pixelX, pixelY, ucCharBuffer[i], MAGENTA, MAGENTA, 1);
            pixelX += 8;
            if (pixelX > 127) {
                pixelX = 0;
                pixelY += 8;
            }
        }

//        drawChar(pixelX, pixelY, 'G', MAGENTA, MAGENTA, 1);
    } else {
        Report("should never run this\n\r");
        for (i = 0; i < letter_count; i++) {
                drawChar(pixelX, pixelY, dad[i], MAGENTA, MAGENTA, 1);
            pixelX += 8;
            if (pixelX > 127) {
                pixelX = 0;
                pixelY += 8;
            }
        }
    }
    globalX = 0;
    globalY = 70;

}

void updateChar(char letter, unsigned int color, int draw, int confirmPrint) {
    // append letter to board + increment X
    fillRect(globalX, globalY-8, 8, 8, BLACK);
    if (draw) {
        drawChar(globalX, globalY-8, letter, color, color, 1);
        if ((same_button_counter == 0) || (pressed_button == 0))
            letter_count++;
        else {
            letter_count = letter_count;
        }
        dad[letter_count - 1] = letter;
        if (confirmPrint){
            globalX += 8;
            if (globalX > 127) {
                globalX = 0;
                globalY += 8;
            }
        }
    } else {
        int boundX = globalX - 8; int boundY = globalY;
        // letter_count--;
        // check if there is a letter to erase
        if (!(boundX == 0 && boundY == 128)) {
            // erase last letter from above row
            if (boundX < 0) {
                boundX = 120;
                boundY -= 8;
            }
            fillRect(boundX, boundY-8, 8, 8, BLACK);

            globalX = boundX;
            globalY = boundY;
        }
    }
    Report("Current message: %.*s\n\r", letter_count, dad);
}

void PrintAndClearTextString() {
    fillScreen(BLACK);
    Report("Final letter count: %d\n\r", letter_count);
//    Report("FINAL Text message: %.*s\n\r", letter_count, text);
    Report("FINAL Text message: %.*s\n\r", letter_count, dad);
    //int i = 0;
//    for (i = 0; i < letter_count; i++) {
//        UARTCharPut(UARTA1_BASE, dad[i]);
//    }
    printText(0);
//    letter_count = 0;
//    memset(dad, 0, sizeof(dad));
//    memset(text, 0, sizeof(text));
}

void SetUpForHTTPPost() {
    PrintAndClearTextString();
    char json_template[] = "{\"state\": {\r\n\"desired\" : {\r\n\"var\" : \"%s\"\r\n}}}\r\n\r\n";
    int length_of_aws_string = snprintf(NULL, 0, json_template, dad);
    char aws_string[length_of_aws_string + 1];
    snprintf(aws_string, sizeof(aws_string), json_template, dad);
    //Report("AWS String: %s\n\r", aws_string);
    long lRetVal = -1;

    //Connect the CC3200 to the local access point
    lRetVal = connectToAccessPoint();
    //Set time so that encryption can be used
    lRetVal = set_time();
    if(lRetVal < 0) {
        UART_PRINT("Unable to set time in the device");
        LOOP_FOREVER();
    }
    //Connect to the website with TLS encryption
    lRetVal = tls_connect();
    if(lRetVal < 0) {
        ERR_PRINT(lRetVal);
    }
    //Report("I AM HERE\n\r");
    http_post(lRetVal, aws_string);
    //http_get(lRetVal);

    sl_Stop(SL_STOP_TIMEOUT);
    LOOP_FOREVER();
    //Report("I AM HERE\n\r");
    letter_count = 0;
    memset(dad, 0, sizeof(dad));
    memset(text, 0, sizeof(text));
}


//void PrintAndClearReceivedTextString() {
//    fillScreen(BLACK);
////    Report("FINAL Text message: %.*s\n\r", letter_count, text);
//    Report("FINAL RECEIVED Text message: %.*s\n\r", ui16CharCounter, ucCharBuffer);
//
//    printText(1);
//    ui16CharCounter = 0;
////    memset(ucCharBuffer, 0, sizeof(ucCharBuffer));
//}

void PrintPressedButton() {
    if (strcmp(data, ARRAY_0) == 0) { Message("You Pressed 0.\n\r"); }
    else if (strcmp(data, ARRAY_1) == 0) { Message("You Pressed 1.\n\r"); }
    else if (strcmp(data, ARRAY_2) == 0) { Message("You Pressed 2.\n\r"); }
    else if (strcmp(data, ARRAY_3) == 0) { Message("You Pressed 3.\n\r"); }
    else if (strcmp(data, ARRAY_4) == 0) { Message("You Pressed 4.\n\r"); }
    else if (strcmp(data, ARRAY_5) == 0) { Message("You Pressed 5.\n\r"); }
    else if (strcmp(data, ARRAY_6) == 0) { Message("You Pressed 6.\n\r"); }
    else if (strcmp(data, ARRAY_7) == 0) { Message("You Pressed 7.\n\r"); }
    else if (strcmp(data, ARRAY_8) == 0) { Message("You Pressed 8.\n\r"); }
    else if (strcmp(data, ARRAY_9) == 0) { Message("You Pressed 9.\n\r"); }
    else if (strcmp(data, ARRAY_LAST) == 0) { Message("You Pressed LAST.\n\r"); }
    else if (strcmp(data, ARRAY_MUTE) == 0) { Message("You Pressed MUTE.\n\r"); }
    return;
}

void DetectOverwrite() {
    //Report("Letter count = %d\n\r", letter_count);
    if (same_button_counter == 0) {
        //Report("text[%d] = %c\n\r", letter_count, text[letter_count]);
        updateChar(text[letter_count], MAGENTA, 1, 1);
        //letter_count++;
    } else {
        //Report("text[%d]-1 = %c\n\r", letter_count, text[letter_count]-1);
        //letter_count--;
        updateChar(text[letter_count]-1, MAGENTA, 0, 0);
        //letter_count--;
        updateChar(text[letter_count], MAGENTA, 1, 1);
    }
}

void CheckMultiTap() {
    switch (pressed_button) {
        case 0:
            updateChar(' ', MAGENTA, 1, 1);
            text[letter_count] = ' ';
            break;
        case 2:
            DetectOverwrite();
            break;
        case 3:
            DetectOverwrite();
            break;
        case 4:
            DetectOverwrite();
            break;
        case 5:
            DetectOverwrite();
            break;
        case 6:
            DetectOverwrite();
            break;
        case 7:
            DetectOverwrite();
            break;
        case 8:
            DetectOverwrite();
            break;
        case 9:
            DetectOverwrite();
            break;
        case 10: // LAST -> our delete for now
            text[letter_count] = 0;
            letter_count --;
            updateChar(' ', MAGENTA, 0, 0);
            break;
        case 11: // MUTE -> for entering the string
            //PrintAndClearTextString();
            SetUpForHTTPPost();
            return;
        default:
            break;

    }
    return;
}

void updateButtonPress() {
    time_t curr_pressed_time;
    curr_pressed_time = time(NULL);
    interval = difftime(curr_pressed_time, prev_button_pressed_time);

    if (pressed_button == prev && interval <= 2 && ((pressed_button == 9 && same_button_counter < 3)
          || (pressed_button == 7 && same_button_counter < 3)  || same_button_counter < 2)) {
        same_button_counter++;
    } else {
        same_button_counter = 0;
    }

    switch (pressed_button) {
        case 2:
            text[letter_count] = 'A' + same_button_counter;
            break;
        case 3:
            text[letter_count] = 'D' + same_button_counter;
            break;
        case 4:
            text[letter_count] = 'G' + same_button_counter;
            break;
        case 5:
            text[letter_count] = 'J' + same_button_counter;
            break;
        case 6:
            text[letter_count] = 'M' + same_button_counter;
            break;
        case 7:
            text[letter_count] = 'P' + same_button_counter;
            break;
        case 8:
            text[letter_count] = 'T' + same_button_counter;
            break;
        case 9:
            text[letter_count] = 'W' + same_button_counter;
            break;
        default:
            break;
    }
    prev_button_pressed_time = curr_pressed_time;
}

void SetPressedNumber() {
    if (letter_count == 0) {
        prev_button_pressed_time = time(NULL);
    }
    if (strcmp(data, ARRAY_0) == 0) { pressed_button = 0; }
    else if (strcmp(data, ARRAY_2) == 0) { pressed_button = 2; updateButtonPress();}
    else if (strcmp(data, ARRAY_3) == 0) { pressed_button = 3; updateButtonPress();}
    else if (strcmp(data, ARRAY_4) == 0) { pressed_button = 4; updateButtonPress();}
    else if (strcmp(data, ARRAY_5) == 0) { pressed_button = 5; updateButtonPress();}
    else if (strcmp(data, ARRAY_6) == 0) { pressed_button = 6; updateButtonPress();}
    else if (strcmp(data, ARRAY_7) == 0) { pressed_button = 7; updateButtonPress();}
    else if (strcmp(data, ARRAY_8) == 0) { pressed_button = 8; updateButtonPress();}
    else if (strcmp(data, ARRAY_9) == 0) { pressed_button = 9; updateButtonPress();}
    else if (strcmp(data, ARRAY_LAST) == 0) { pressed_button = 10; }
    else if (strcmp(data, ARRAY_MUTE) == 0) { pressed_button = 11; }
    else { return; }
    prev = pressed_button;

    CheckMultiTap();
}

void PrintMeaningfulInfo() {
        //int i = 0;
//        for (i = 0; i < 33; i++) {
//            Report("systick_get[%d] = %llu\t systick_get_ms[%d] = %.3f\t",
//                   i, systick_get[i], i, systick_get_ms[i]);
//            if (i < 17) {
//                Report("bit: %c\n\r", start_and_address[i]);
//            } else {
//                Report("bit: %c\n\r", data[i-17]);
//            }
//        }
//        Report("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r");
        PrintPressedButton();
        ClearArrays();
}

void GetMeaningfulInfo() {
    SetPressedNumber();
    ClearArrays();
}

void printtextapp() {
    drawChar(15, 64, 'T', YELLOW, YELLOW, 2);
    drawChar(25, 64, 'e', YELLOW, YELLOW, 2);
    drawChar(35, 64, 'x', YELLOW, YELLOW, 2);
    drawChar(45, 64, 't', YELLOW, YELLOW, 2);
    drawChar(55, 64, ' ', YELLOW, YELLOW, 2);
    drawChar(65, 64, 'A', YELLOW, YELLOW, 2);
    drawChar(75, 64, 'p', YELLOW, YELLOW, 2);
    drawChar(85, 64, 'p', YELLOW, YELLOW, 2);
}

//void
//InitTerm_0()
//{
//#ifndef NOTERM
//  MAP_UARTConfigSetExpClk(BOARD_TO_BOARD,MAP_PRCMPeripheralClockGet(BOARD_TO_BOARD_PERIPH),
//                  UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
//                   UART_CONFIG_PAR_NONE));
//#endif
//  __Errorlog = 0;
//}
//
//static void UARTIntHandler() {
//    uart_int_count++;
//    unsigned long ulStatus;
//    ulStatus = MAP_UARTIntStatus(UARTA1_BASE, 1);
//    MAP_UARTIntClear(UARTA1_BASE, ulStatus);
//    while (UARTCharsAvail(UARTA1_BASE)) {
//        ucCharBuffer[ui16CharCounter] = UARTCharGet(UARTA1_BASE);
//        ui16CharCounter++;
//    }
//}

//*****************************************************************************
//
//! Main 
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void main() {
    //long lRetVal = -1;

    unsigned long ulStatus;
    //
    // Initialize board configuration
    //
    BoardInit();

    PinMuxConfig();

    // Enable SysTick
    SysTickInit();

    InitTerm();
    ClearTerm();
    UART_PRINT("My terminal works!\n\r");

    //
    // Register the interrupt handlers
    //
    MAP_GPIOIntRegister(GPIOA3_BASE, GPIOA3IntHandler);

    //
    // Configure falling edge interrupts on IR
    //
    MAP_GPIOIntTypeSet(GPIOA3_BASE, 0x10, GPIO_FALLING_EDGE);    // IR

    ulStatus = MAP_GPIOIntStatus (GPIOA3_BASE, false);
    MAP_GPIOIntClear(GPIOA3_BASE, ulStatus);            // clear interrupts on GPIOA3


    // clear global variables
    IR_intcount=0;
    IR_intflag=0;
    state = 0;

    // Enable IR interrupts
    MAP_GPIOIntEnable(GPIOA3_BASE, 0x10);
    MasterMain();
    Adafruit_Init();
    fillScreen(BLACK);
    printtextapp();
    delay(10); fillScreen(BLACK);

    Message("\t\t****************************************************\n\r");
    Message("\t\tIR Interrupt\n\r");
    Message("\t\t ****************************************************\n\r");
    Message("\n\n\n\r");

//    //Connect the CC3200 to the local access point
//   lRetVal = connectToAccessPoint();
//   //Set time so that encryption can be used
//   lRetVal = set_time();
//   if(lRetVal < 0) {
//       UART_PRINT("Unable to set time in the device");
//       LOOP_FOREVER();
//   }
//   //Connect to the website with TLS encryption
//   lRetVal = tls_connect();
//   if(lRetVal < 0) {
//       ERR_PRINT(lRetVal);
//   }
//   Report("I AM HERE\n\r");

    while (1) {
        while ((IR_intflag==0) /*&& (UART_RX_intflag == 0)*/) {
//            if (ui16CharCounter > 0) {
//                Report("FINAL while loop Received Text message: %.*s\n\r", ui16CharCounter, ucCharBuffer);
//                PrintAndClearReceivedTextString();
//            }
            ;
        }
        if ((IR_intflag)) {
            IR_intflag=0;  // clear flag
            //PrintPressedButton();
            SetPressedNumber();
            IR_intcount = 0;
        }
    }

//    sl_Stop(SL_STOP_TIMEOUT);
//    LOOP_FOREVER();
//    //Connect the CC3200 to the local access point
//    lRetVal = connectToAccessPoint();
//    //Set time so that encryption can be used
//    lRetVal = set_time();
//    if(lRetVal < 0) {
//        UART_PRINT("Unable to set time in the device");
//        LOOP_FOREVER();
//    }
//    //Connect to the website with TLS encryption
//    lRetVal = tls_connect();
//    if(lRetVal < 0) {
//        ERR_PRINT(lRetVal);
//    }
//    http_post(lRetVal);
//    //http_get(lRetVal);
//
//    sl_Stop(SL_STOP_TIMEOUT);
//    LOOP_FOREVER();
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

static int http_post(int iTLSSockID, char *AWSString) {
    //Report("AFSDASDFADSFFFADSAFDSDFSA");
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, POSTHEADER);
    pcBufHeaders += strlen(POSTHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    int dataLength = strlen(AWSString);

    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);

    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);

    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);

//    strcpy(pcBufHeaders, DATA1);
//    pcBufHeaders += strlen(DATA1);

    strcpy(pcBufHeaders, AWSString);
    pcBufHeaders += strlen(AWSString);

    int testDataLength = strlen(pcBufHeaders);

    UART_PRINT(acSendBuff);


    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    UART_PRINT("lRetVal 1 = %i\n\r", lRetVal);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    UART_PRINT("lRetVal 2 = %i\n\r", lRetVal);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    return 0;
}

static int http_get(int iTLSSockID) {
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, GETHEADER);
    pcBufHeaders += strlen(GETHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    UART_PRINT(acSendBuff);

    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    UART_PRINT("lRetVal = %i\n\r", lRetVal);
    if(lRetVal < 0) {
        UART_PRINT("GET failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    UART_PRINT("lRetVal = %i\n\r", lRetVal);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    return 0;
}
