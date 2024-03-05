#include "includes.h"
#include "macros.h"
#include "globals.h"

void I2CCode() {
    //
        // I2C Init
        //
        I2C_IF_Open(I2C_MASTER_MODE_FST);

        int XPosition = 63; int YPosition = 63;
        int radius = 2;
        unsigned char data_buffer[128];
        signed char new_X; signed char new_Y;


        Adafruit_Init();
        fillScreen(BLACK);
        while(1) {
            I2C_IF_Write(SLAVE_ADDRESS, &reg_offset, 1, 0);
            I2C_IF_Read(SLAVE_ADDRESS, &data_buffer[0], RD_LEN);
            UART_PRINT("In Get X and Y\n\r");
            new_X = (signed char)data_buffer[1]; new_Y = (signed char)data_buffer[3];
            UART_PRINT("x after: 0x%02X\n", new_X);
            UART_PRINT("y after: 0x%02X\n", new_Y);
            fillCircle(XPosition, YPosition, radius, BLACK);
            XPosition += new_X/8; YPosition += new_Y/8;
            if (XPosition > 127 - radius) {
                XPosition = 127 - radius;
            }
            if (XPosition < 0 + radius) {
                XPosition = 0 + radius;
            }
            if (YPosition > 127 - radius) {
                YPosition = 127 - radius;
            }
            if (YPosition < 0 + radius) {
                YPosition = 0 + radius;
            }
            UART_PRINT("new_X: 0x%02X\n", new_X);
            UART_PRINT("new_Y: 0x%02X\n", new_Y);
            fillCircle(XPosition, YPosition, radius, YELLOW);
            delay(1);
        }

    }
