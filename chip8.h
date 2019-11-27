#include "sdl.h"

#define START_LOCATION 0x200
#define MAX_GAME_SIZE (0x1000 - START_LOCATION)

#define MEMORY_SIZE 4096
#define REGISTER_SIZE 16
#define STACK_SIZE 16

#define SPRITE_WIDTH 8

#define true 1
#define false 0
#define bool int

#define ROOMS_FOLDER "rooms"

void chip8_initialize(void);
void chip8_loadgame(char *game);
void chip8_emulatecycle(void);
void chip8_setkeys(void);
void chip8_tick(void);
