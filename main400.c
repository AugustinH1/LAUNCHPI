#include "include/button.h"
#include "lib/libINET/data.h"

#include <ncurses.h>
#include <stdlib.h>

//CONFIGURATION RESEAU
#define PORT_SVC 5000
#define IP "192.168.190.204"

#define ROW 4
#define COL 4

int selected_row = 0;
int selected_col = 0;


void display_matrix(button *matrix);
void modify_frequency(button *matrix);
void serialize_matrix(button *matrix, char *buffer);
void deserialize_matrix(char *buffer, button *matrix);


int main() {
    //création de la socket de dialogue
    socket_t sockDialogue;
    sockDialogue = connecterClt2Srv(IP, PORT_SVC);

    

    button matrix;
    initButtonMatrix(&matrix);

    // Initialiser ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

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

    //fermeture de la socket de dialogue
    close(sockDialogue.fd);


    // Terminer ncurses
    endwin();
    return 0;
}



void display_matrix(button *matrix) {
    clear();
    for (int i = 0; i < ROW; ++i) {
        for (int j = 0; j < COL; ++j) {
            if (i == selected_row && j == selected_col) {
                attron(A_REVERSE);
                mvprintw(i, j * 10, "%.2f", matrix->frequencies[i][j]);
                attroff(A_REVERSE);
            } else {
                mvprintw(i, j * 10, "%.2f", matrix->frequencies[i][j]);
            }
        }
    }
    // Afficher le menu récapitulatif en bas
    mvprintw(ROW + 1, 0, "Utilisez les flèches pour naviguer. Appuyez sur Entrée pour modifier une fréquence.");
    mvprintw(ROW + 2, 0, "Appuyez sur 'q' pour quitter.");
    refresh();
}

void modify_frequency(button *matrix) {
    echo();  // Activer l'affichage de la saisie utilisateur
    mvprintw(ROW + 3, 0, "Entrez la nouvelle fréquence pour le bouton sélectionné : ");
    refresh();
    
    char input[100];
    getstr(input);  // Attendre que l'utilisateur entre une chaîne de caractères
    
    matrix->frequencies[selected_row][selected_col] = atof(input);  // Convertir et stocker
    
    noecho();  // Désactiver l'affichage de la saisie utilisateur
    display_matrix(matrix);  // Mettre à jour l'affichage de la matrice
}

void serialize_matrix(button *matrix, char *buffer) {
    int offset = 0;
    int i,j;
    for (i = 0; i < ROW; ++i) {
        for (j = 0; j < COL; ++j) {
            offset += sprintf(buffer + offset, "%.2f ", matrix->frequencies[i][j]);
        }
    }
}


void deserialize_matrix(char *buffer, button *matrix) {
    int offset = 0;
    int i,j;
    for (i = 0; i < ROW; ++i) {
        for (j = 0; j < COL; ++j) {
            sscanf(buffer + offset, "%lf", &matrix->frequencies[i][j]);
            offset += 6;
        }
    }
}