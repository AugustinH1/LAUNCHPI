#include <bcm2835.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#define uchar unsigned char
#define uint unsigned int


#define Max7219_pinCS  RPI_GPIO_P1_26

// Redéfinir la macro delayMicroseconds pour éviter les conflits de nom
#ifdef delayMicroseconds
#undef delayMicroseconds
#endif
#define bcm2835_delayMicroseconds bcm2835_delayMicroseconds_original

// Définition conditionnelle pour éviter les conflits de macros
#ifdef HIGH
#undef HIGH
#endif

#ifdef LOW
#undef LOW
#endif

#include <wiringPi.h>



void Delay_xms(uint x);
void Write_Max7219_byte(uchar DATA);
void Write_Max7219(uchar address1,uchar dat1,uchar address2,uchar dat2);
void Init_MAX7219();
void led_print(int index);
void *launchMatrix();
int initLedMatrix();


extern uchar disp[9][8];