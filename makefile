PATH_CC?=tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin

CCC?=$(PATH_CC)/arm-linux-gnueabihf-gcc

AR?=$(PATH_CC)/arm-linux-gnueabihf-ar

IP_JOYPI?=192.168.190.204
IP_400?=

CFLAGS = -Wall -Wextra
TARGET_RPI =-L target_rpi/lib -I target_rpi/include
CC = gcc
ARM = $(CCC)

all:

buttonPC.o: lib/button.c
	$(CC) $(CFLAGS) -c lib/button.c -o lib/buttonPC.o

sessionPC.o: lib/libINET/session.c
	$(CC) $(CFLAGS) -c lib/libINET/session.c -o lib/sessionPC.o
dataPC.o: lib/libINET/data.c
	$(CC) $(CFLAGS) -c lib/libINET/data.c -o lib/dataPC.o
libINETPC.a: sessionPC.o dataPC.o
	$(AR) -q lib/libINETPC.a lib/sessionPC.o lib/dataPC.o
	rm -f lib/sessionPC.o lib/dataPC.o

main400PC.exe: main400.c buttonPC.o libINETPC.a
	$(CC) $(CFLAGS) main400.c lib/buttonPC.o -o main400PC.exe -lm -lncurses -L lib -lINETPC




# COMPILATION POUR LE RPI


sessionRPI.o: lib/libINET/session.c
	$(ARM) $(CFLAGS) $(TARGET_RPI) -c lib/libINET/session.c -o lib/sessionRPI.o

dataRPI.o: lib/libINET/data.c
	$(ARM) $(CFLAGS) $(TARGET_RPI) -c lib/libINET/data.c -o lib/dataRPI.o
libINETRPI.a: sessionRPI.o dataRPI.o
	$(AR) -q lib/libINETRPI.a lib/sessionRPI.o lib/dataRPI.o
	rm -f lib/sessionRPI.o lib/dataRPI.o


soundRPI.o: lib/sound.c
	$(ARM) $(CFLAGS) $(TARGET_RPI) -c lib/sound.c -o lib/soundRPI.o

buttonRPI.o: lib/button.c
	$(ARM) $(CFLAGS) $(TARGET_RPI) -c lib/button.c -o lib/buttonRPI.o

mainJoyPiRpi.exe: mainJoyPi.c soundRPI.o buttonRPI.o libINETRPI.a
	$(ARM) $(CFLAGS) $(TARGET_RPI) mainJoyPi.c lib/soundRPI.o lib/buttonRPI.o -o mainJoyPiRpi.exe -l wiringPi -lasound -lm -lpthread -L lib -lINETRPI



# INSTALLATION PAR SSH

installJoyPi: mainJoyPiRpi.exe
	sshpass -ppi scp mainJoyPiRpi.exe pi@$(IP_JOYPI):/home/pi/

install400: main400Rpi.exe
	sshpass -ppi scp main400Rpi.exe pi@$(IP_400):/home/pi/

install: installJoyPi install400


clean:
	rm -f *.exe lib/*.o lib/*.a