#include "include/sound.h"
#include "include/button.h"
#include "include/matrix.h"
#include "lib/libINET/data.h"
#include <wiringPi.h>
#include <pthread.h>

//CONFIGURATION RESEAU
#define PORT_SVC 5000
//mettre l'ip automatiquement
#define IP "192.168.4.1"

//GETION DES PIN
#define ROW 4
#define COL 4

#define SENSITIVE_BUTTON 0
#define VIBRATION 2

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

uchar disp[9][8] = {
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //Blank
    {
		0b00000000,
		0b01100000,
		0b10010000,
		0b10010000,
		0b00001001,
		0b00001001,
		0b00000110,
		0b00000000
	},//sin1
	{
		0b00000000,
		0b11000000,
		0b00100001,
		0b00100001,
		0b00010010,
		0b00010010,
		0b00001100,
		0b00000000
	},//sin2
	{
		0b00000000,
		0b10000001,
		0b01000010,
		0b01000010,
		0b00100100,
		0b00100100,
		0b00011000,
		0b00000000
	},
	{
		0b00000000,
		0b00000011,
		0b10000100,
		0b10000100,
		0b01001000,
		0b01001000,
		0b00110000,
		0b00000000
	},
	{
		0b00000000,
		0b00000110,
		0b00001001,
		0b00001001,
		0b10010000,
		0b10010000,
		0b01100000,
		0b00000000
	},
	{
		0b00000000,
		0b00001100,
		0b00010010,
		0b00010010,
		0b00100001,
		0b00100001,
		0b11000000,
		0b00000000
	},
	{
		0b00000000,
		0b00011000,
		0b00100100,
		0b00100100,
		0b01000010,
		0b01000010,
		0b10000001,
		0b00000000
	},
	{
		0b00000000,
		0b00110000,
		0b01001000,
		0b01001000,
		0b10000100,
		0b10000100,
		0b00000011,
		0b00000000
	}

};

int main(void) {
    
    snd_pcm_t *handle;

    //initialiser wirpingpi
    setup();
    //initialiser la matrice de bouton et de fréquence
    initButtonMatrix(&matrix, ROW, COL);
    initLedMatrix();
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
        play_tones(handle, frequencies, num_frequencies, 0.1);
    }

    // Arrêter et fermer le périphérique audio
    snd_pcm_drain(handle);
    snd_pcm_close(handle);


    return EXIT_SUCCESS;
}

//THREAD
void *getMatriceInet(){
    while(1){
        socket_t sockEcoute;
        sockEcoute = creerSocketEcoute(IP, PORT_SVC);

        socket_t sockDialogue;
        sockDialogue = accepterClt(sockEcoute);
        printf("ip client: %s\n", inet_ntoa(sockDialogue.addrDst.sin_addr));

        close(sockEcoute.fd);

        envoyer(&sockDialogue, &matrix, (pFct)serialize_matrix);

        while(1){
            //recevoir la matrice
            //button matrice;
            button tmp;
            recevoir(&sockDialogue, &tmp, (pFct)deserialize_matrix);

            //Afficher
            int i,j;
            int flag = 0;
            for (i = 0; i < ROW; i++) {
                for (j = 0; j < COL; j++) {
                    printf("%.2f ", tmp.frequencies[i][j]);
                    if(tmp.frequencies[i][j] == -1){
                        flag = 1;
                    }
                }
                printf("\n");
            }        
            
            if(flag == 1){
                printf("fin de la communication\n");
                break;
            }

            //copier les valeurs de frequence dans la matrice
            memcpy(&matrix.frequencies, &tmp.frequencies, sizeof(tmp.frequencies));
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

    pinMode(SENSITIVE_BUTTON, INPUT);
    pullUpDnControl(SENSITIVE_BUTTON, PUD_DOWN);
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

    if(digitalRead(SENSITIVE_BUTTON) == HIGH)
        digitalWrite(VIBRATION, HIGH);
    else
        digitalWrite(VIBRATION, LOW);

    

    // Copier l'état temporaire dans la structure matrix
    memcpy(matrix->active, tmpActive, sizeof(tmpActive));
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
