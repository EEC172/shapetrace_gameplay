#include "includes.h"
#include "macros.h"
#include "globals.h"

//Global Declarations
int globalX = 0; int globalY = 70;
//char expectedX[128];
//char expectedY[128];
//Coordinate expected_coordinates[MAX_SIZE];
int expected_pixel_count = 0;
char expected[ROWS][COLS];

//void ResetXAndY();
//void SetXAndYCoordinate(int, int);
//void MasterMain();
//void PrintCoverPage();
//void printText(int);
//void updateChar(char, unsigned int, int, int);
//void DrawSquareAndUpdatePixelArray();
//void DrawTriangleAndUpdateArray();

void ResetXAndY() {
//    memset(expectedX, '0', sizeof(expectedX));
//    memset(expectedY, '0', sizeof(expectedY));
    memset(expected, '0', sizeof(expected));
    expected_pixel_count = 0;
}

void SetXAndYCoordinate(int x, int y) {
    //expectedX[x] = '1'; expectedY[y] = '1';
//    expected_coordinates[expected_pixel_count].coordinateX = x;
//    expected_coordinates[expected_pixel_count].coordinateY = y;
    expected[x][y] = '1';
    expected_pixel_count++;
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

void PrintConstantString(const char *fixed_string, int x_prompt, int y_prompt, int color) {
    int i = 0;
    for (i = 0; i < strlen(fixed_string); i++) {
        drawChar(x_prompt, y_prompt, fixed_string[i], color, color, 1);
        x_prompt += 6;
        if (x_prompt > 127) {
            x_prompt = 0; y_prompt += 8;
        }
    }
}

void PrintCoverPage() {
    fillRect(0, 0, 128, 128, DARK_BLUE);
    drawChar(10, 64, 'S', YELLOW, YELLOW, 2);
    drawChar(20, 64, 'h', YELLOW, YELLOW, 2);
    drawChar(30, 64, 'a', YELLOW, YELLOW, 2);
    drawChar(40, 64, 'p', YELLOW, YELLOW, 2);
    drawChar(50, 64, 'e', YELLOW, YELLOW, 2);
    drawChar(60, 64, 'T', YELLOW, YELLOW, 2);
    drawChar(70, 64, 'r', YELLOW, YELLOW, 2);
    drawChar(80, 64, 'a', YELLOW, YELLOW, 2);
    drawChar(90, 64, 'c', YELLOW, YELLOW, 2);
    drawChar(100, 64, 'e', YELLOW, YELLOW, 2);

    const char *init_prompt = "Press 0 on IR to      continue";
    PrintConstantString(init_prompt, 0, 100, WHITE);
}

void PrintInstructions() {
    fillRect(0, 0, 128, 128, DARK_BLUE);
    const char *orientation_prompt = "Orient CC3200         horizontally and place above OLED.";
    PrintConstantString(orientation_prompt, 0, 0, YELLOW);
    const char *tilt_instruction = "You will trace over a predrawn shape and    get an accuracy score.";
    PrintConstantString(tilt_instruction, 0, 35, YELLOW);
    const char *leaderboard = "Top accuracy scores   will display on a     leaderboard";
    PrintConstantString(leaderboard, 0, 70, YELLOW);
    const char *prompt = "Press 0 to advance";
    PrintConstantString(prompt, 0, 115, WHITE);
}

void ShapeOptions() {
    fillRect(0, 0, 128, 128, DARK_BLUE);
    const char *square = "SQUARE -> PRESS 2";
    PrintConstantString(square, 0, 20, CYAN);
    const char *triangle = "TRIANGLE -> PRESS 3";
    PrintConstantString(triangle, 0, 60, CYAN);
    const char *circle = "CIRCLE -> PRESS 4";
    PrintConstantString(circle, 0, 100, CYAN);
}

void OtherRules() {
    fillRect(0, 0, 128, 128, DARK_BLUE);
    const char *multitap_heading = "MULTITAP (Press 7):";
    PrintConstantString(multitap_heading, 0, 0, CYAN);
    const char *tap1 = "Press 7 1x - Yellow";
    const char *tap2 = "Press 7 2x - Red";
    const char *tap3 = "Press 7 3x - Magenta";
    const char *tap4 = "Press 7 4x - Green";
    PrintConstantString(tap1, 0, 10, YELLOW);
    PrintConstantString(tap2, 0, 20, RED);
    PrintConstantString(tap3, 0, 30, MAGENTA);
    PrintConstantString(tap4, 0, 40, GREEN);

    const char *mute = "FINISH - Press MUTE";
    const char *last = "RESTART - Press LAST";
    PrintConstantString(mute, 0, 65, CYAN);
    PrintConstantString(last, 0, 90, CYAN);

    const char *prompt = "Press 0 to advance";
    PrintConstantString(prompt, 0, 115, WHITE);
}

void drawCompass() {
    // horizontal line
    drawLine(10, 0, 10, 20, CYAN);
    // horizontal arrow left
    drawLine(8, 18, 10, 20, CYAN);
    drawLine(12, 18, 10, 20, CYAN);
    // horizontal arrow right
    drawLine(8, 2, 10, 0, CYAN);
    drawLine(12, 2, 10, 0, CYAN);

    // vertical
    drawLine(0, 10, 20, 10, CYAN);
    // vertical arrow up
    drawLine(2, 12, 0, 10, CYAN);
    drawLine(2, 8, 0, 10, CYAN);
    // vertical arrow down
    drawLine(18, 12, 20, 10, CYAN);
    drawLine(18, 8, 20, 10, CYAN);

}

//void printText(int type_array) {
//    int pixelX = 0;
//    int pixelY = 0;
//    int i = 0;
//    // don't let x or y go past 128
//    if (type_array) {
//        Report("PRINT FUNC ui16 Char Count: %d\n\r", ui16CharCounter);
//        Report("FINAL PRINT FUNC Received Text message: %.*s\n\r", ui16CharCounter, ucCharBuffer);
//
//        for (i = 0; i < ui16CharCounter; i++) {
//            drawChar(pixelX, pixelY, ucCharBuffer[i], MAGENTA, MAGENTA, 1);
//            pixelX += 8;
//            if (pixelX > 127) {
//                pixelX = 0;
//                pixelY += 8;
//            }
//        }
//    } else {
//        Report("should never run this\n\r");
//        for (i = 0; i < letter_count; i++) {
//                drawChar(pixelX, pixelY, dad[i], MAGENTA, MAGENTA, 1);
//            pixelX += 8;
//            if (pixelX > 127) {
//                pixelX = 0;
//                pixelY += 8;
//            }
//        }
//    }
//    globalX = 0;
//    globalY = 70;
//
//}
//
//void updateChar(char letter, unsigned int color, int draw, int confirmPrint) {
//    // append letter to board + increment X
//    fillRect(globalX, globalY-8, 8, 8, BLACK);
//    if (draw) {
//        drawChar(globalX, globalY-8, letter, color, color, 1);
//        if ((same_button_counter == 0) || (pressed_button == 0))
//            letter_count++;
//        else {
//            letter_count = letter_count;
//        }
//        dad[letter_count - 1] = letter;
//        if (confirmPrint){
//            globalX += 8;
//            if (globalX > 127) {
//                globalX = 0;
//                globalY += 8;
//            }
//        }
//    } else {
//        int boundX = globalX - 8; int boundY = globalY;
//        // check if there is a letter to erase
//        if (!(boundX == 0 && boundY == 128)) {
//            // erase last letter from above row
//            if (boundX < 0) {
//                boundX = 120;
//                boundY -= 8;
//            }
//            fillRect(boundX, boundY-8, 8, 8, BLACK);
//
//            globalX = boundX;
//            globalY = boundY;
//        }
//    }
//    Report("Current message: %.*s\n\r", letter_count, dad);
//}

void DrawSquareAndUpdateArray() {
    ResetXAndY();
    fillRect(0, 0, 128, 128, DARK_BLUE);
    //expected_pixel_count = 0;
    int starting_point = 30;
    int x = starting_point; int y = starting_point;
    int limit = starting_point + SQUARE_SIZE;
    for (y = starting_point; y < limit; y++) {
        for (x = starting_point; x < limit; x++) {
            if (y == starting_point || y == limit-1 || x == starting_point || x == limit-1) {
                fillCircle(x, y, 1,CYAN);
                SetXAndYCoordinate(x, y);
            }
        }
    }
}

void DrawTriangleAndUpdateArray() {
    ResetXAndY();
    fillRect(0, 0, 128, 128, DARK_BLUE);
    int base = 60; int height = 60; int starting_point = 30;
    int x = starting_point; int y = starting_point;
    for (y = starting_point; y < starting_point + height; y++) {
        for (x = starting_point; x < starting_point + base - (y - 30); x++) {
            if (x == starting_point || y == starting_point ||
                    x == starting_point + base - (y - 30) - 1) {
                fillCircle(x, y, 1, CYAN);
                SetXAndYCoordinate(x, y);
            }
        }
    }
}

void DrawCircleAndUpdateArray(int x0, int y0, int r, unsigned int color) {
      ResetXAndY();
      fillRect(0, 0, 128, 128, DARK_BLUE);
      int f = 1 - r;
      int ddF_x = 1;
      int ddF_y = -2 * r;
      int x = 0;
      int y = r;

      fillCircle(x0  , y0+r, 1, color);
      SetXAndYCoordinate(x0, y0+r);
      fillCircle(x0  , y0-r, 1, color);
      SetXAndYCoordinate(x0, y0-r);
      fillCircle(x0+r, y0, 1, color);
      SetXAndYCoordinate(x0+r, y0);
      fillCircle(x0-r, y0, 1, color);
      SetXAndYCoordinate(x0-r, y0);

      while (x<y) {
        if (f >= 0) {
          y--;
          ddF_y += 2;
          f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        fillCircle(x0 + x, y0 + y, 1, color);
        SetXAndYCoordinate(x0+x, y0+y);
        fillCircle(x0 - x, y0 + y, 1, color);
        SetXAndYCoordinate(x0-x, y0+y);
        fillCircle(x0 + x, y0 - y, 1, color);
        SetXAndYCoordinate(x0+x, y0-y);
        fillCircle(x0 - x, y0 - y, 1, color);
        SetXAndYCoordinate(x0-x, y0-y);
        fillCircle(x0 + y, y0 + x, 1, color);
        SetXAndYCoordinate(x0+y, y0+x);
        fillCircle(x0 - y, y0 + x, 1, color);
        SetXAndYCoordinate(x0-y, y0+x);
        fillCircle(x0 + y, y0 - x, 1, color);
        SetXAndYCoordinate(x0+y, y0-x);
        fillCircle(x0 - y, y0 - x, 1, color);
        SetXAndYCoordinate(x0-y, y0-x);
    }
    //Report("Current count: %d\n", expected_pixel_count);
}
