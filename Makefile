CXX=gcc
CFLAGS=-g -Wall -o3
LFLAGS=-lm # -lws2811

all: libws2811.a main

libws2811.a: rpihw.c dma.c pwm.c pcm.c ws2811.c mailbox.c
	gcc -c rpihw.c dma.c pwm.c pcm.c ws2811.c mailbox.c -O3
	ar rcs libws2811.a rpihw.o dma.o pwm.o pcm.o ws2811.o mailbox.o

main: libws2811.a
	gcc -g -Wall -L. main.c -O3 -o main -lws2811 -lm

clean:
	rm -rf main
	rm -rf *.dSYM
	rm -rf *.a
	rm -rf *.o
