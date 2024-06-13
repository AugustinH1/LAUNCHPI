#include "../include/matrix.h"

void Delay_xms(uint x)
{
	bcm2835_delay(x);
}
//------------------------

void Write_Max7219_byte(uchar DATA)
{
	bcm2835_gpio_write(Max7219_pinCS,LOW);
	bcm2835_spi_transfer(DATA);
}

void Write_Max7219(uchar address1,uchar dat1,uchar address2,uchar dat2)
{
	bcm2835_gpio_write(Max7219_pinCS,LOW);
	Write_Max7219_byte(address1);
	Write_Max7219_byte(dat1); 
	Write_Max7219_byte(address2);
	Write_Max7219_byte(dat2);
	bcm2835_gpio_write(Max7219_pinCS,HIGH);
}

void Init_MAX7219()
{
	Write_Max7219(0x09,0x00,0x09,0x00);
	Write_Max7219(0x0a,0x03,0x0a,0x03);
	Write_Max7219(0x0b,0x07,0x0b,0x07);
	Write_Max7219(0x0c,0x01,0x0c,0x01);
	Write_Max7219(0x0f,0x00,0x0f,0x00);
}


void led_print(int index) {
	uchar i;
	for(i = 1;i < 9;i++) {
		Write_Max7219(i, 0,i,disp[index][i-1]);
	}
}

void *launchMatrix() {
	if (!bcm2835_init()) {
		printf("Unable to init bcm2835.\n");
		pthread_exit(NULL);
	}

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256); // The default
	bcm2835_gpio_fsel(Max7219_pinCS, BCM2835_GPIO_FSEL_OUTP); 
	bcm2835_gpio_write(disp[0][0],HIGH);
	Delay_xms(50);
	Init_MAX7219();

	int i;
	while(1){
		for(i = 1; i < 8; i++) {
			led_print(i);
			Delay_xms(200);
		}
	}

}

//initLedMatrix(); doit lancer un thread qui affiche sur la matrice de LED
int initLedMatrix() {
	pthread_t threadMatrix;
	pthread_create(&threadMatrix, NULL, launchMatrix, NULL);
	return 0;
}
