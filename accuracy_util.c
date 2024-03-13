#include "macros.h"
#include "includes.h"
#include "globals.h"

float max_points = 0.0;
float user_points = 0.0;
float accuracy_percentage = 0.0;

void GenerateAccuracy() {
    max_points = (float)expected_pixel_count;
    int expectedX, expectedY; //actualX, actualY;
    if (actual_pixel_count > expected_pixel_count) {
        for (expectedY = 0; expectedY < COLS; expectedY++) {
            for (expectedX = 0; expectedX < ROWS; expectedX++) {
                if (expected[expectedX][expectedY] == '1' ) {
                    if (actual[expectedX][expectedY] == '1') {
                        user_points += 1;
                    }
                }
            }
        }
    }
    accuracy_percentage = (user_points/max_points) * 100.00;
    Report("Accuracy is: %.2f\n\r", accuracy_percentage);
}
