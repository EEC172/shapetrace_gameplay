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

#include "macros.h"
#include "includes.h"
#include "globals.h"
#include "common.h"


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

static unsigned long __Errorlog;
//*****************************************************************************
//                 GLOBAL VARIABLES -- End: df
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static void BoardInit(void);

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

/**
 * SysTick Interrupt Handler
 *
 * Keep track of whether the systick counter wrapped
 */
static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt++;
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

void PrintAndClearTextString() {
    fillScreen(BLACK);
    Report("Final letter count: %d\n\r", letter_count);
    Report("FINAL Text message: %.*s\n\r", letter_count, dad);
    printText(0);
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
    http_post(lRetVal, aws_string);

    sl_Stop(SL_STOP_TIMEOUT);
    LOOP_FOREVER();
    letter_count = 0;
    memset(dad, 0, sizeof(dad));
    memset(text, 0, sizeof(text));
}

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
    if (same_button_counter == 0) {
        updateChar(text[letter_count], MAGENTA, 1, 1);
    } else {
        updateChar(text[letter_count]-1, MAGENTA, 0, 0);
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
            letter_count--;
            updateChar(' ', MAGENTA, 0, 0);
            break;
        case 11: // MUTE -> for entering the string
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
    PrintCoverPage();
    delay(10); fillScreen(BLACK);

    Message("\t\t****************************************************\n\r");
    Message("\t\tIR Interrupt\n\r");
    Message("\t\t ****************************************************\n\r");
    Message("\n\n\n\r");

    while (1) {
        while ((IR_intflag==0)) {;}
        if ((IR_intflag)) {
            IR_intflag=0;  // clear flag
            //PrintPressedButton();
            SetPressedNumber();
            IR_intcount = 0;
        }
    }
}

