#include "includes.h"
#include "macros.h"
#include "globals.h"

//Global Declarations
void MasterMain();
void PrintCoverPage();
void printText(int);
void updateChar(char, unsigned int, int, int);

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

void PrintCoverPage() {
    drawChar(10, 64, 'S', YELLOW, CYAN, 2);
    drawChar(20, 64, 'h', YELLOW, CYAN, 2);
    drawChar(30, 64, 'a', YELLOW, CYAN, 2);
    drawChar(40, 64, 'p', YELLOW, CYAN, 2);
    drawChar(50, 64, 'e', YELLOW, CYAN, 2);
    drawChar(60, 64, 'T', YELLOW, CYAN, 2);
    drawChar(70, 64, 'r', YELLOW, CYAN, 2);
    drawChar(80, 64, 'a', YELLOW, CYAN, 2);
    drawChar(90, 64, 'c', YELLOW, CYAN, 2);
    drawChar(100, 64, 'e', YELLOW, CYAN, 2);
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
            drawChar(pixelX, pixelY, ucCharBuffer[i], MAGENTA, MAGENTA, 1);
            pixelX += 8;
            if (pixelX > 127) {
                pixelX = 0;
                pixelY += 8;
            }
        }
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
