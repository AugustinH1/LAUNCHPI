#include "include/button.h"
#include "lib/libINET/data.h"

#include <ncurses.h>
#include <stdlib.h>

//CONFIGURATION RESEAU
#define PORT_SVC 5000
#define IP "192.168.4.1"

#define ROW 4
#define COL 4

int selected_row = 0;
int selected_col = 0;


void display_matrix(button *matrix);
void modify_frequency(button *matrix);
void serialize_matrix(button *matrix, char *buffer);
void deserialize_matrix(char *buffer, button *matrix);


int main() {
    // Création de la socket de dialogue
    socket_t sockDialogue;
    sockDialogue = connecterClt2Srv(IP, PORT_SVC);

    button matrix;

    recevoir(&sockDialogue, &matrix, (pFct)deserialize_matrix);

    // Initialiser ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();  // Activer les couleurs
    init_pair(1, COLOR_BLACK, COLOR_WHITE);  // Couleurs pour la sélection

    display_matrix(&matrix);

    int ch;
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                selected_row = (selected_row - 1 + ROW) % ROW;
                break;
            case KEY_DOWN:
                selected_row = (selected_row + 1) % ROW;
                break;
            case KEY_LEFT:
                selected_col = (selected_col - 1 + COL) % COL;
                break;
            case KEY_RIGHT:
                selected_col = (selected_col + 1) % COL;
                break;
            case '\n':
                modify_frequency(&matrix);
                envoyer(&sockDialogue, &matrix, (pFct)serialize_matrix);
                break;
        }
        display_matrix(&matrix);
    }
    
    // Envoyer la matrice où tout est à -1
    matrix.frequencies[0][0] = -1;
    envoyer(&sockDialogue, &matrix, (pFct)serialize_matrix);

    // Fermeture de la socket de dialogue
    close(sockDialogue.fd);

    // Terminer ncurses
    endwin();
    return 0;
}

void display_matrix(button *matrix) {
    clear();

    // Calculer la position de départ pour centrer la matrice
    int start_row = (LINES - ROW) / 2;
    int start_col = (COLS - COL * 10) / 2;

    for (int i = 0; i < ROW; ++i) {
        for (int j = 0; j < COL; ++j) {
            if (i == selected_row && j == selected_col) {
                attron(COLOR_PAIR(1));
                mvprintw(start_row + i, start_col + j * 10, "%.0f", matrix->frequencies[i][j]);
                attroff(COLOR_PAIR(1));
            } else {
                mvprintw(start_row + i, start_col + j * 10, "%.0f", matrix->frequencies[i][j]);
            }
        }
    }

    // Afficher le menu récapitulatif en bas
    mvprintw(LINES - 2, 0, "Utilisez les flèches pour naviguer. Appuyez sur Entrée pour modifier une fréquence.");
    mvprintw(LINES - 1, 0, "Appuyez sur 'q' pour quitter.");

    refresh();
}

void modify_frequency(button *matrix) {
    echo();  // Activer l'affichage de la saisie utilisateur
    mvprintw(LINES - 4, 0, "Entrez la nouvelle fréquence pour le bouton [%d, %d] : ", selected_row, selected_col);
    clrtoeol();  // Effacer la ligne après le message
    refresh();
    while(1) {    
        char input[100];
        getstr(input);  // Attendre que l'utilisateur entre une chaîne de caractères

        // Si valeur négative, redemander la saisie
        if (atof(input) < 0) {
            mvprintw(LINES - 3, 0, "Valeur négative non autorisée");
            clrtoeol();
            refresh();
        } else {
            matrix->frequencies[selected_row][selected_col] = atof(input);  // Convertir et stocker
            break;
        }
    }
    
    noecho();  // Désactiver l'affichage de la saisie utilisateur
    display_matrix(matrix);  // Mettre à jour l'affichage de la matrice
}

void serialize_matrix(button *matrix, char *buffer) {
    sprintf(buffer, "%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f", 
    matrix->frequencies[0][0], matrix->frequencies[0][1], matrix->frequencies[0][2], matrix->frequencies[0][3],
    matrix->frequencies[1][0], matrix->frequencies[1][1], matrix->frequencies[1][2], matrix->frequencies[1][3],
    matrix->frequencies[2][0], matrix->frequencies[2][1], matrix->frequencies[2][2], matrix->frequencies[2][3],
    matrix->frequencies[3][0], matrix->frequencies[3][1], matrix->frequencies[3][2], matrix->frequencies[3][3]);
}

void deserialize_matrix(char *buffer, button *matrix) {
    sscanf(buffer, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
    &matrix->frequencies[0][0], &matrix->frequencies[0][1], &matrix->frequencies[0][2], &matrix->frequencies[0][3],
    &matrix->frequencies[1][0], &matrix->frequencies[1][1], &matrix->frequencies[1][2], &matrix->frequencies[1][3],
    &matrix->frequencies[2][0], &matrix->frequencies[2][1], &matrix->frequencies[2][2], &matrix->frequencies[2][3],
    &matrix->frequencies[3][0], &matrix->frequencies[3][1], &matrix->frequencies[3][2], &matrix->frequencies[3][3]);
}
