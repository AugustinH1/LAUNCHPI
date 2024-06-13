#include <stdio.h>
#include <string.h>
#include <math.h>

// Définir les broches pour les lignes et les colonnes
#define ROW 4
#define COL 4

typedef struct {
    double frequencies[ROW][COL];
    int active[ROW][COL];
    int vibreur;
} button;

void initButtonMatrix(button *matrix, int row, int col);