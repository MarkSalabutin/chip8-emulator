#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define SCALE 20

#define KEY_SIZE 16

#define PIXEL_OFF 0x000000
#define PIXEL_ON 0xFFFFFF

void sdl_initgraphics(void);
void sdl_drawgraphics(void);
void sdl_cleanup(void);
void sdl_clearscreen(void);
int sdl_initaudio(void);
void sdl_updateaudio(int);
