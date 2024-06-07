#include "../include/button.h"

void initButtonMatrix(button *matrix) {
    double baseFrequency = 440.0;
    int i,j;
    for (i = 0; i < ROW; i++) {
        for (j = 0; j < COL; j++) {
            matrix->frequencies[i][j] = baseFrequency * pow(2, (double) (i * COL + j) / 12.0);
            matrix->active[i][j] = 0;
        }
    }
    memset(matrix->active, 0, sizeof(matrix->active));
}