CC = gcc
CFLAGS = -Wall -Wextra
#LDFLAGS = -D DEBUG

all: libINET.a

session.o: session.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c session.c

data.o: data.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c data.c

libINET.a: session.o data.o
	ar -q libINET.a session.o data.o
	make clean

clean:
	rm -f *.o

mrproprer: clean
	rm -f libINET.a