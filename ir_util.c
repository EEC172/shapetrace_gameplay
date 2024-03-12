#include "macros.h"
#include "includes.h"
#include "globals.h"
#include "common.h"

volatile uint64_t delta;
volatile double delta_ms;

//// track systick counter periods elapsed
//// if it is not 0, we know the transmission ended
volatile int systick_cnt = 0;
volatile unsigned long IR_intcount;
volatile unsigned char IR_intflag;

volatile unsigned char setup_flag;

volatile int shape_type = 0;

uint64_t systick_get[33];
double systick_get_ms[33];
char start_and_address[17]; char data[16];
int letter_count = 0;
char text[64];
int pressed_button = 0; int prev = -1; int same_button_counter = 0;
time_t prev_button_pressed_time;
time_t uart_handler_time;
double interval;

char dad[64];

unsigned char ucCharBuffer[64];
uint16_t ui16CharCounter = 0;
int uart_int_count = 0;
int UART_RX_intflag = 0;


void ClearArrays(void) {
    if (IR_intcount >= 33) {
        memset(systick_get, 0, sizeof(systick_get));
        memset(systick_get_ms, 0, sizeof(systick_get_ms));
        memset(start_and_address, 0, sizeof(start_and_address));
        memset(data, 0, sizeof(data));
    }
}

bool ConfirmStartAndAddress() { return (strcmp(start_and_address, START_ADDRESS) == 0); }

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
            //updateChar(' ', MAGENTA, 1, 1);
            //text[letter_count] = ' ';
            break;
        case 2:
            //DetectOverwrite();
            break;
        case 3:
            //DetectOverwrite();
            break;
        case 4:
            //DetectOverwrite();
            break;
        case 5:
            //DetectOverwrite();
            break;
        case 6:
            //DetectOverwrite();
            break;
        case 7:
            //DetectOverwrite();
            break;
        case 8:
            //DetectOverwrite();
            break;
        case 9:
            //DetectOverwrite();
            break;
        case 10: // LAST -> our delete for now
            //text[letter_count] = 0;
            //letter_count--;
            //updateChar(' ', MAGENTA, 0, 0);
            //fillScreen(BLACK);
            break;
        case 11: // MUTE -> for entering the string
            //Report("In mute in multitap\n\r");
            //SetUpForHTTPPost();
            break;
        default:
            break;

    }
    return;
}

void updateButtonPress() {
    time_t curr_pressed_time;
    curr_pressed_time = time(NULL);
    interval = difftime(curr_pressed_time, prev_button_pressed_time);

    if (pressed_button == 7 && pressed_button == prev && interval <= 2 &&
            ((pressed_button == 9 && same_button_counter < 3) || (pressed_button == 7 && same_button_counter < 3)
                    || same_button_counter < 2)) {
        same_button_counter++;
    } else {
        same_button_counter = 0;
    }

//    switch (pressed_button) {
//        case 2:
//            text[letter_count] = 'A' + same_button_counter;
//            break;
//        case 3:
//            text[letter_count] = 'D' + same_button_counter;
//            break;
//        case 4:
//            text[letter_count] = 'G' + same_button_counter;
//            break;
//        case 5:
//            text[letter_count] = 'J' + same_button_counter;
//            break;
//        case 6:
//            text[letter_count] = 'M' + same_button_counter;
//            break;
//        case 7:
//            text[letter_count] = 'P' + same_button_counter;
//            break;
//        case 8:
//            text[letter_count] = 'T' + same_button_counter;
//            break;
//        case 9:
//            text[letter_count] = 'W' + same_button_counter;
//            break;
//        default:
//            break;
//    }
    prev_button_pressed_time = curr_pressed_time;
}

void SetPressedNumber() {
    if (letter_count == 0) {
        prev_button_pressed_time = time(NULL);
    }
    if (strcmp(data, ARRAY_0) == 0) { pressed_button = 0; setup_flag = 1; }
    else if (strcmp(data, ARRAY_2) == 0) { pressed_button = 2; updateButtonPress(); }
    else if (strcmp(data, ARRAY_3) == 0) { pressed_button = 3; updateButtonPress(); }
    else if (strcmp(data, ARRAY_4) == 0) { pressed_button = 4; updateButtonPress(); }
//    else if (strcmp(data, ARRAY_5) == 0) { pressed_button = 5; updateButtonPress(); }
//    else if (strcmp(data, ARRAY_6) == 0) { pressed_button = 6; updateButtonPress(); }
    else if (strcmp(data, ARRAY_7) == 0) { pressed_button = 7; updateButtonPress(); }
    else if (strcmp(data, ARRAY_8) == 0) { pressed_button = 8; updateButtonPress(); }
    else if (strcmp(data, ARRAY_9) == 0) { pressed_button = 9; updateButtonPress(); }
    else if (strcmp(data, ARRAY_LAST) == 0) { pressed_button = 10; }
    else if (strcmp(data, ARRAY_MUTE) == 0) { pressed_button = 11; }
    else { return; }
    prev = pressed_button;
    Report("%d\n\r", pressed_button);

    CheckMultiTap();
}

void PrintAndClearTextString() {
    fillScreen(BLACK);
    Report("Final letter count: %d\n\r", letter_count);
    Report("FINAL Text message: %.*s\n\r", letter_count, dad);
    printText(0);
}

void GetMeaningfulInfo() {
    SetPressedNumber();
    ClearArrays();
}

void IRGameSetup() {
    while (setup_flag == 0) {
        while ((IR_intflag==0)) {;}
        if ((IR_intflag)) {
            IR_intflag=0;  // clear flag
            SetPressedNumber(); IR_intcount = 0;
            if (setup_flag == 1) {
                setup_flag = 0;
                fillScreen(BLACK);
                break;
            }
        }
    }
}

void IRRemoteOptionSetup() {
    int chosen_button = 0; //temp variable
    while (chosen_button == 0) {
        while ((IR_intflag==0)) {;}
        if (IR_intflag) {
            IR_intflag=0;  // clear flag
            SetPressedNumber(); IR_intcount = 0;
            chosen_button = pressed_button;
            if (chosen_button == 2) {
                fillScreen(BLACK);
                globalX = 30; globalY = 30;
                DrawSquareAndUpdateArray();
                chosen_button = 0; break;
            } else if (chosen_button == 3) {
                fillScreen(BLACK);
                globalX = 30; globalY = 30;
                DrawTriangleAndUpdateArray();
                chosen_button = 0; break;
            } else if (chosen_button == 4) {
                fillScreen(BLACK);
                globalX = 60; globalY = 30;
                DrawCircleAndUpdateArray(60, 60, 30, CYAN);
                chosen_button = 0; break;
            }
        }
    }
}


//void PrintPressedButton() {
//    if (strcmp(data, ARRAY_0) == 0) { Message("You Pressed 0.\n\r"); }
//    else if (strcmp(data, ARRAY_1) == 0) { Message("You Pressed 1.\n\r"); }
//    else if (strcmp(data, ARRAY_2) == 0) { Message("You Pressed 2.\n\r"); }
//    else if (strcmp(data, ARRAY_3) == 0) { Message("You Pressed 3.\n\r"); }
//    else if (strcmp(data, ARRAY_4) == 0) { Message("You Pressed 4.\n\r"); }
//    else if (strcmp(data, ARRAY_5) == 0) { Message("You Pressed 5.\n\r"); }
//    else if (strcmp(data, ARRAY_6) == 0) { Message("You Pressed 6.\n\r"); }
//    else if (strcmp(data, ARRAY_7) == 0) { Message("You Pressed 7.\n\r"); }
//    else if (strcmp(data, ARRAY_8) == 0) { Message("You Pressed 8.\n\r"); }
//    else if (strcmp(data, ARRAY_9) == 0) { Message("You Pressed 9.\n\r"); }
//    else if (strcmp(data, ARRAY_LAST) == 0) { Message("You Pressed LAST.\n\r"); }
//    else if (strcmp(data, ARRAY_MUTE) == 0) { Message("You Pressed MUTE.\n\r"); }
//    return;
//}

//void PrintMeaningfulInfo() {
//        //int i = 0;
////        for (i = 0; i < 33; i++) {
////            Report("systick_get[%d] = %llu\t systick_get_ms[%d] = %.3f\t",
////                   i, systick_get[i], i, systick_get_ms[i]);
////            if (i < 17) {
////                Report("bit: %c\n\r", start_and_address[i]);
////            } else {
////                Report("bit: %c\n\r", data[i-17]);
////            }
////        }
////        Report("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r");
//        PrintPressedButton();
//        ClearArrays();
//}
