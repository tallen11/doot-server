CXX=gcc
CFLAGS=-g -Wall
LFLAGS= # -lws2811

all: libws2811.a main

libws2811.a: rpihw.c dma.c pwm.c pcm.c ws2811.c mailbox.c
	gcc -c rpihw.c dma.c pwm.c pcm.c ws2811.c mailbox.c
	ar rcs libws2811.a rpihw.o dma.o pwm.o pcm.o ws2811.o mailbox.o

main: libws2811.a
	gcc -g -Wall -L. main.c -o main -lws2811

clean:
	rm -rf main
	rm -rf *.dSYM
	rm -rf *.a
	rm -rf *.o
