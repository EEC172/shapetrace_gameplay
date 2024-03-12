#include "includes.h"
#include "macros.h"
#include "globals.h"

unsigned char reg_offset = 0x02;
//Coordinate actual_coordinates[5000];
int actual_pixel_count = 0;

//void ResetActualCoordinates() {
//    memset(actual_coordinates, 0, sizeof(actual_coordinates));
//    actual_pixel_count = 0;
//}
//
//void UpdateActualCoordinates(int x, int y) {
//    actual_coordinates[actual_pixel_count].coordinateX = x;
//    actual_coordinates[actual_pixel_count].coordinateY = y;
//    Report("X: %d\t Y: %d\t", actual_coordinates[actual_pixel_count].coordinateX,
//           actual_coordinates[actual_pixel_count].coordinateY);
//    actual_pixel_count++;
//}

void I2CCode() {
    //
    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);

    int XPosition = globalX; int YPosition = globalY;
    int radius = 1;
    fillCircle(XPosition, YPosition, radius, YELLOW);
    unsigned char data_buffer[128];
    signed char new_X; signed char new_Y;
    drawCompass();

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

            fillCircle(XPosition, YPosition, radius, YELLOW);
//            UpdateActualCoordinates(XPosition, YPosition);
//            Report("Actual Pixel Count: %d\n\r", actual_pixel_count);
            modifyRowsBit(XPosition); modifyColsBit(YPosition);

        }
        if (IR_intflag) {

            // sometimes doesn't register the MUTE button, just post anways
//            SetPressedNumber(); IR_intcount = 0;
            fillScreen(BLACK); // replace with ending screen + score
            SetUpForHTTPPost();

            break;

        }

        delay(1);
    }


}
