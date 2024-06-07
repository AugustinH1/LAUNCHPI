#include <stdio.h>
#include <string.h>
#include <math.h>

// Définir les broches pour les lignes et les colonnes
#define ROW 4
#define COL 4
extern int rowPins[ROW];
extern int colPins[COL];

typedef struct {
    double frequencies[ROW][COL];
    int active[ROW][COL];
} button;

void initButtonMatrix(button *matrix);