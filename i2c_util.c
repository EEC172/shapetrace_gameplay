#include "includes.h"
#include "macros.h"
#include "globals.h"

unsigned char reg_offset = 0x02;
int actual_pixel_count = 0;
char actual[ROWS][COLS];

void ResetActualCoordinates() {
    memset(actual, 0, sizeof(actual));
    actual_pixel_count = 0;
}
//
void UpdateActualCoordinates(int x, int y) {
    actual[x][y] = '1';
    actual_pixel_count++;
    Report("X: %d\t Y: %d\t count: %d\n\r", x, y, actual_pixel_count);
}

void I2CCode() {
    //
    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);
    memset(actual, '0', sizeof(actual));

    int XPosition = globalX; int YPosition = globalY;
    int radius = 1; unsigned int color = YELLOW;
    fillCircle(XPosition, YPosition, radius, color);
    UpdateActualCoordinates(XPosition, YPosition);
    unsigned char data_buffer[128];
    signed char new_X; signed char new_Y;
    drawCompass();
    int chosen_button = 0;

    while(1) {
        I2C_IF_Write(SLAVE_ADDRESS, &reg_offset, 1, 0);
        I2C_IF_Read(SLAVE_ADDRESS, &data_buffer[0], RD_LEN);
        new_X = (signed char)data_buffer[3];
        new_Y = (signed char)data_buffer[1];

        if ((new_X/8) == 1 || (new_X/8) == -1 ||
                (new_Y/8) == 1 || (new_Y/8) == -1) {
            XPosition += new_X/8;
            YPosition += new_Y/8;

            if (XPosition > 127 - radius) {
                XPosition = 127 - radius;
            }
            if (YPosition > 127 - radius) {
                YPosition = 127 - radius;
            }

            if (XPosition <= 25 + radius && YPosition <= 25 + radius ) {
                XPosition -= new_X/8;
                YPosition -= new_Y/8;
            }


            if (XPosition < 0 + radius) {
                XPosition = 0 + radius;
            }
            if (YPosition < 0 + radius) {
                YPosition = 0 + radius;
            }

            fillCircle(XPosition, YPosition, radius, color);
            UpdateActualCoordinates(XPosition, YPosition);
            //Report("X: %d\t Y: %d\t count: %d\n\r", XPosition, YPosition, count);
            //count++;
//            UpdateActualCoordinates(XPosition, YPosition);
//            Report("Actual Pixel Count: %d\n\r", actual_pixel_count);
            //modifyRowsBit(XPosition); modifyColsBit(YPosition);

        }
        if (IR_intflag) {
            IR_intflag=0;
            // sometimes doesn't register the MUTE button, just post anways
            SetPressedNumber(); IR_intcount = 0;
            Report("same_button_counter: %d\n\r", same_button_counter);
            chosen_button = pressed_button;
            if (chosen_button == 11) {
                fillScreen(BLACK); // replace with ending screen + score
                GenerateAccuracy();
                //fillScreen(BLACK);
                SetUpForHTTPPost();
                fillScreen(BLACK);
                same_button_counter = 0;
                break;
            } else if (chosen_button == 10) {
                fillScreen(BLACK);
                same_button_counter = 0;
                memset(actual, 0, sizeof(actual));
                memset(expected, 0, sizeof(expected));
                memset(rows, 0, sizeof(rows));
                memset(cols, 0, sizeof(cols));
                //memset(text, 0, sizeof(text));
                break;
            }
            if (same_button_counter == 1) { color = RED; }
            else if (same_button_counter == 2) { color = MAGENTA; }
            else if (same_button_counter == 3) { color = GREEN; }
        }

        delay(5);
    }


}
