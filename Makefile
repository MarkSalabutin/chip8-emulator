CC=gcc
CFLAGS=-O3 -Wall

FRAMEWORKS=-framework SDL2 -F /Library/Frameworks/

all: chip8

chip8: main.c chip8.c sdl.c
	${CC} ${FRAMEWORKS} $^ -o $@
  
clean:
	rm -f chip8
