#include "../include/button.h"

void initButtonMatrix(button *matrix, int row, int col) {
    double baseFrequency = 440.0;
    int i,j;
    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
            matrix->frequencies[i][j] = baseFrequency * pow(2, (double) (i * col + j) / 12.0);
            matrix->active[i][j] = 0;
        }
    }
    memset(matrix->active, 0, sizeof(matrix->active));
}