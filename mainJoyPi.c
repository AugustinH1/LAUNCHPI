#include "include/sound.h"
#include "include/button.h"
#include "lib/libINET/data.h"
#include <wiringPi.h>
#include <pthread.h>

//CONFIGURATION RESEAU
#define PORT_SVC 5000
//mettre l'ip automatiquement
#define IP "192.168.190.204"

//GETION DES PIN
#define ROW 4
#define COL 4

int rowPins[ROW] = {2, 3, 21, 22};   // Broches GPIO pour les lignes // pin 13, 15 29 31
int colPins[COL] = {6, 25, 24, 23};  // Broches GPIO pour les colonnes // pin 22 37 35 33


//SETUP
void setup();
void readButtonMatrix(button *matrix);
void serialize_matrix(button *matrix, char *buffer);
void deserialize_matrix(char *buffer, button *matrix);
void *getMatriceInet();


button matrix;



//VARIABLE GLOBALE
double frequencies[100];
int num_frequencies = 0;

int main(void) {
    
    snd_pcm_t *handle;

    //initialiser wirpingpi
    setup();
    //initialiser la matrice de bouton et de fréquence
    initButtonMatrix(&matrix);
    // Initialiser le périphérique audio
    init_audio(&handle);

    //initialiser le thread
    pthread_t threadInet;
    pthread_create(&threadInet, NULL, getMatriceInet, NULL);

    
    while (1) {

        //LIRE LES BOUTONS
        readButtonMatrix(&matrix);
        num_frequencies = 0;
        int i,j;

        // Mise a plat des fréquences actives
        for (i = 0; i < ROW; i++) {
            for (j = 0; j < COL; j++) {
                if (matrix.active[i][j] == 1) {
                    printf("%f, ", matrix.frequencies[i][j]);
                    frequencies[num_frequencies++] = matrix.frequencies[i][j];
                }
            }

        }
        if (num_frequencies > 0) {
            printf("\n" );
        }
        
        //JOUER LE SON
        play_tones(handle, frequencies, num_frequencies);
    }

    // Arrêter et fermer le périphérique audio
    snd_pcm_drain(handle);
    snd_pcm_close(handle);


    return EXIT_SUCCESS;
}

//THREAD
void *getMatriceInet(){
    //ouvrir socket ecoute sur le port 5000
    //accepter la connection
    //boucle infinie de reception de matrice
    //fermer la conne
    //retour au debut du thread
    while(1){
        socket_t sockEcoute;
        sockEcoute = creerSocketEcoute(IP, PORT_SVC);

        socket_t sockDialogue;
        sockDialogue = accepterClt(sockEcoute);
        printf("ip client: %s\n", inet_ntoa(sockDialogue.addrDst.sin_addr));

        close(sockEcoute.fd);

        while(1){
            //recevoir la matrice
            //button matrice;
            button tmp;
            recevoir(&sockDialogue, &tmp, (pFct)deserialize_matrix);

            //copier les valeurs de frequence dans la matrice
            memcpy(&matrix.frequencies, &tmp.frequencies, sizeof(tmp.frequencies));

            //Afficher
            int i,j;
            for (i = 0; i < ROW; i++) {
                for (j = 0; j < COL; j++) {
                    printf("%.2f ", matrix.frequencies[i][j]);
                }
                printf("\n");
            }        
            

    
            
        }
        close(sockDialogue.fd);
    }
}

void setup() {
    wiringPiSetup();
    int i;
    // Configurer les broches des lignes comme sorties
    for (i = 0; i < ROW; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH);  // Mettre les lignes à HIGH initialement
    }

    // Configurer les broches des colonnes comme entrées avec pull-up
    for (i = 0; i < COL; i++) {
        pinMode(colPins[i], INPUT);
        pullUpDnControl(colPins[i], PUD_UP);
    }
}

void readButtonMatrix(button *matrix) {
    // Variable temporaire pour stocker l'état des boutons
    int tmpActive[ROW][COL];
    memset(tmpActive, 0, sizeof(tmpActive));

    int row, col;
    for (row = 0; row < ROW; row++) {
        digitalWrite(rowPins[row], LOW);

        for (col = 0; col < COL; col++) {
            int buttonState = digitalRead(colPins[col]);
            if (buttonState == LOW) {
                tmpActive[row][col] = 1;
            }
        }

        digitalWrite(rowPins[row], HIGH);

    }

    // Copier l'état temporaire dans la structure matrix
    memcpy(matrix->active, tmpActive, sizeof(tmpActive));
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
