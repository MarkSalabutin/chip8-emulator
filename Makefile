CC=gcc
CFLAGS=-O3 -Wall

LIBS=-lSDL2

all: chip8

chip8: main.c chip8.c sdl.c
	${CC} $^ ${LIBS} -o $@
  
clean:
	rm -f chip8
