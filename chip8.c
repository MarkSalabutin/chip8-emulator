#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "chip8.h"

static inline uint8_t randbyte() { return (rand() % 256); }

extern uint32_t gfx[DISPLAY_WIDTH * DISPLAY_HEIGHT];

static const SDL_Scancode keymap[KEY_SIZE] = {
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
    SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
    SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V
};

uint16_t opcode;
uint16_t PC;
uint16_t I;
uint16_t SP;

uint8_t delay_timer;
uint8_t sound_timer;

uint8_t memory[MEMORY_SIZE];
uint8_t V[REGISTER_SIZE];
uint16_t stack[STACK_SIZE];
uint16_t key[KEY_SIZE];

bool mute = false;
bool draw_flag;

#define FONTSET_ADDRESS 0x00
#define FONTSET_BYTES_PER_CHAR 5
unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_initialize() {
    sdl_initgraphics();
    
    if (!sdl_initaudio()) {
        printf("Could not enable sound.\n");
        mute = true;
    }

    PC = START_LOCATION;
    opcode = 0;
    I = 0;
    SP = 0;

    memset(memory, 0, sizeof(uint8_t)  * MEMORY_SIZE);
    memset(V, 0, sizeof(uint8_t) * REGISTER_SIZE);
    memset(stack, 0, sizeof(uint16_t) * STACK_SIZE);
    memset(key, 0, sizeof(uint16_t) * KEY_SIZE);
    sdl_clearscreen();

    for (int i = 0; i < 80; i++)
        memory[FONTSET_ADDRESS + i] = chip8_fontset[i];

    draw_flag = true;
    delay_timer = 0;
    sound_timer = 0;
    
    time_t t;
    srand((unsigned) time(&t));
}

void chip8_emulatecycle() {
    int i;
    
    uint8_t x, y, n, kk;
    uint16_t nnn;
  
    opcode = memory[PC] << 8 | memory[PC+1];
    x = (opcode >> 8) & 0x000F; // the lower 4 bits of high byte
    y = (opcode >> 4) & 0x000F; // the upper four bits of low byte
    n = opcode & 0x000F; // the lowest 4 bits
    kk = opcode & 0x00FF; // the lowest 8 bits
    nnn = opcode & 0x0FFF; // the lowest 12 bits

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (kk) {
                case 0x00E0: // clear the screen
                    sdl_clearscreen();
                    draw_flag = true;
                    PC+=2;
                    break;
                case 0x00EE: // return
                    PC = stack[--SP];
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    exit(42);
            }
            break;
        case 0x1000: // 1nnn: jump to address nnn
            PC = nnn;
            break;
        case 0x2000: // 2nnn: call address nnn
            stack[SP++] = PC + 2;
            PC = nnn;
            break;
        case 0x3000: // 3xkk: skip the next instruction if V[x] == kk
            PC += (V[x] == kk) ? 4 : 2;
            break;
        case 0x4000: // 4xkk: skip the next instruction if V[x] != kk
            PC += (V[x] != kk) ? 4 : 2;
            break;
        case 0x5000: // 5xy0: skip the next instruction if V[x] == V[y]
            PC += (V[x] == V[y]) ? 4 : 2;
            break;
        case 0x6000: // 6xkk: set V[x] = kk
            V[x] = kk;
            PC += 2;
            break;
        case 0x7000: // 7xkk: set V[x] = V[x] + kk
            V[x] += kk;
            PC += 2;
            break;
        case 0x8000: // 8xyn: arithmetics
            switch (n) {
                case 0x0:
                    V[x] = V[y];
                    break;
                case 0x1:
                    V[x] = V[x] | V[y];
                    break;
                case 0x2:
                    V[x] = V[x] & V[y];
                    break;
                case 0x3:
                    V[x] = V[x] ^ V[y];
                    break;
                case 0x4:
                    V[0xF] = ((int)V[x] + (int)V[y]) > 255 ? 1 : 0;
                    V[x] = V[x] + V[y];
                    break;
                case 0x5:
                    V[0xF] = (V[x] > V[y]) ? 1 : 0;
                    V[x] = V[x] - V[y];
                    break;
                case 0x6:
                    V[0xF] = V[x] & 0x1;
                    V[x] = V[x] >> 1;
                    break;
                case 0x7:
                    V[0xF] = (V[y] > V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    break;
                case 0xE:
                    V[0xF] = (V[x] >> 7) & 0x1;
                    V[x] = V[x] << 1;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    exit(42);
            }
            PC += 2;
            break;
        case 0x9000:
            switch (n) {
                case 0x0: // 9xy0: skip the next instruction if V[x] != V[y]
                    PC += (V[x] != V[y]) ? 4 : 2;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    exit(42);
            }
            break;
        case 0xA000: // Annn: set I to nnn
            I = nnn;
            PC += 2;
            break;
        case 0xB000: // Bnnn: jump to location V[0] + nnn
            PC = V[0] + nnn;
            break;
        case 0xC000: // Cxkk: set V[x] to random byte & kk
            V[x] = randbyte() & kk;
            PC += 2;
            break;
        case 0xD000: // Dxyn:  Display an n-byte sprite starting at memory
                     // location I at (Vx, Vy) on the screen, VF = collision
            {
                uint8_t spriteHeight = n;
                uint8_t pixel;
                V[0xF] = 0x0;
                for(int posY = 0; posY < spriteHeight; posY++) {
                    pixel = memory[I+posY];
                    for (int posX = 0; posX < SPRITE_WIDTH; posX++) {
                        if (pixel & (0x80 >> posX)) {
                            int index = ((V[x] + posX) % DISPLAY_WIDTH) + ((V[y] + posY) % DISPLAY_HEIGHT) * DISPLAY_WIDTH;
                            if (gfx[index] == PIXEL_ON) {
                                V[0xF] = 0x1;
                                gfx[index] = PIXEL_OFF;
                            } else {
                                gfx[index] = PIXEL_ON;
                            }
                        }
                    }
                }
            }
            draw_flag = true;
            PC += 2;
            break;
            
        case 0xE000: // key-pressed events
            switch (kk) {
                case 0x9E: // Ex9E: skip the next instruction if the V[x] key is pressed
                    PC += SDL_GetKeyboardState(NULL)[keymap[V[x]]] ? 4 : 2;
                    break;
                case 0xA1: // ExA1: skip the next instruction if the V[x] key is not pressed
                    PC += !SDL_GetKeyboardState(NULL)[keymap[V[x]]] ? 4 : 2;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    exit(42);
            }
            break;
        case 0xF000:
            switch(kk) {
                case 0x07: // Fx07: set V[x] to delay_timer value
                    V[x] = delay_timer;
                    PC += 2;
                    break;
                case 0x0A: // Fx0A: wait for key instruction
                    for (int i = 0; i < KEY_SIZE; i++) {
                        if (SDL_GetKeyboardState(NULL)[keymap[i]]) {
                            V[x] = i;
                            PC += 2;
                        }
                    }
                    break;
                case 0x15: // Fx15: set delay timer to V[x]
                    delay_timer = V[x];
                    PC += 2;
                    break;
                case 0x18: // Fx18: set sound_timer to V[x]
                    sound_timer = V[x];
                    PC += 2;
                    break;
                case 0x1E:
                    V[0xF] = (I + V[x] > 0xFFF) ? 1 : 0;
                    I = I + V[x];
                    PC += 2;
                    break;
                case 0x29:
                    I = FONTSET_BYTES_PER_CHAR * V[x];
                    PC += 2;
                    break;
                case 0x33:
                    memory[I] = (V[x] % 1000) / 100;
                    memory[I + 1] = (V[x] % 100) / 10;
                    memory[I + 2] = (V[x] % 10);
                    PC += 2;
                    break;
                case 0x55:
                    for (i = 0; i <= x; i++) { memory[I+i] = V[i]; }
                    PC += 2;
                    break;
                case 0x65:
                    for (i = 0; i <= x; i++) { V[i] = memory[I+i]; }
                    PC += 2;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    exit(42);

            }
            break;
    default:
        printf ("Unknown opcode: 0x%X\n", opcode);
        exit(42);
    }
}

void chip8_loadgame(char *game) {
    FILE *fgame;
    
    fgame = fopen(game, "rb");

    if (fgame == NULL) {
        printf("Could not load the game: %s \n", game);
        exit(42);
    }

    fread(&memory[START_LOCATION], 1, MAX_GAME_SIZE, fgame);

    fclose(fgame);
}

void chip8_tick() {
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (mute) {
        return;
    }
    
    if (sound_timer > 0) {
        if (--sound_timer == 0) {
            sdl_updateaudio(0);
        } else {
            sdl_updateaudio(1);
        }
    }
}
