#include "macros.h"
#include "includes.h"
#include "globals.h"

float max_points = 0.0;
float user_points = 0.0;
float accuracy_percentage = 0.0;

void FindClosestActualPoint(int expectedX, int expectedY, int addOrSubtract) {
    int actualY = 0; int actualX = 0; int dy = 0; int dx = 0;
    float closest = ERROR_RANGE;
    for (dy = -4; dy <= 4; dy++) {
        actualY = expectedY + dy;
        if (actualY < 0 || actualY >= COLS) { continue; }
        for (dx = -4; dx <= 4; dx++) {
            actualX = expectedX + dx;
            if (actualY < 0 || actualY >= COLS) { continue; }
            if (actual[actualX][actualY] == '1') {
                float error = sqrt(pow(dx, 2) + pow(dy, 2));
                if (error < closest) {
                    closest = error;
                }
            }
        }
    }
    if (addOrSubtract == SUBTRACT) { user_points -= (closest / (float)ERROR_RANGE); }
    else if (addOrSubtract == ADD) { user_points += (1.0 - (closest / (float)ERROR_RANGE)); }
}

void GenerateAccuracy() {
    max_points = (float)expected_pixel_count;
    Report("Max points: %.2f\n\r", max_points);
    Report("Actual pixel count: %d\n\r", actual_pixel_count);
    int expectedX, expectedY;
    float closest = ERROR_RANGE;
    if (actual_pixel_count >= expected_pixel_count) {
        user_points = max_points;
        for (expectedY = 0; expectedY < COLS; expectedY++) {
            for (expectedX = 0; expectedX < ROWS; expectedX++) {
                if (expected[expectedX][expectedY] == '1' &&
                        actual[expectedX][expectedY] != '1') {
                    FindClosestActualPoint(expectedX, expectedY, SUBTRACT);
                }
            }
        }
    } else {
        user_points = 0;
        for (expectedY = 0; expectedY < COLS; expectedY++) {
            for (expectedX = 0; expectedX < ROWS; expectedX++) {
                if (expected[expectedX][expectedY] == '1' &&
                        actual[expectedX][expectedY] != '1') {
                    FindClosestActualPoint(expectedX, expectedY, ADD);
                } else if (expected[expectedX][expectedY] == '1' &&
                        actual[expectedX][expectedY] == '1') {
                    user_points+=1.0;
                }
            }
        }
    }
    accuracy_percentage = (user_points/max_points);
    Report("Accuracy_percentage before penalty check: %.2f\n\r", accuracy_percentage*100.00);
    if (actual_pixel_count > expected_pixel_count) {
        float penalty = 1.0 - ((float)(actual_pixel_count-expected_pixel_count)/(float)(expected_pixel_count));
        accuracy_percentage = accuracy_percentage * penalty;
        Report("penalty: %.2f\n\r", penalty);
        Report("Expected pixel count: %d\n\r", expected_pixel_count);
        int target = expected_pixel_count * 2;
        if (actual_pixel_count >= target) {
            Report("I am here\n\r");
            accuracy_percentage = 0.0;
        }
    }
    accuracy_percentage = accuracy_percentage * 100.00;
    Report("Accuracy is: %.2f\n\r", accuracy_percentage);
    PrintAccuracy();
    max_points = 0; user_points = 0;
}
