#include <math.h>
#include "sdl.h"

struct audiodata_t {
    float tone_pos;
    float tone_inc;
};

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

SDL_AudioDeviceID audio_device_id;

uint32_t gfx[DISPLAY_WIDTH * DISPLAY_HEIGHT];

void sdl_initgraphics() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("Could not init SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    window = SDL_CreateWindow(
        "CHIP8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE,
        SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (window == NULL) {
        printf("Could not create a window: %s\n", SDL_GetError());
        sdl_cleanup();
        exit(EXIT_FAILURE);
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Could not create a renderer: %s\n", SDL_GetError());
        sdl_cleanup();
        exit(EXIT_FAILURE);
    }
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (texture == NULL) {
        printf("Count not create a texture: %s\n", SDL_GetError());
        sdl_cleanup();
        exit(EXIT_FAILURE);
    }
}

void sdl_drawgraphics() {
    SDL_UpdateTexture(texture, NULL, gfx, DISPLAY_WIDTH * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void sdl_cleanup() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void sdl_clearscreen() {
    memset(gfx, PIXEL_OFF, sizeof(uint32_t) * DISPLAY_WIDTH * DISPLAY_HEIGHT);
}

void printStatus(void)
{
    switch (SDL_GetAudioStatus())
    {
        case SDL_AUDIO_STOPPED: printf("stopped\n"); break;
        case SDL_AUDIO_PLAYING: printf("playing\n"); break;
        case SDL_AUDIO_PAUSED: printf("paused\n"); break;
        default: printf("???"); break;
    }
}

void feed(void* udata, Uint8* stream, int len) {
    struct audiodata_t* audio = (struct audiodata_t *) udata;
    for (int i = 0; i < len; i++) {
        stream[i] = sinf(audio->tone_pos) + 127;
        audio->tone_pos += audio->tone_inc;
    }
}

SDL_AudioSpec* sdl_initaudiospec() {
    struct audiodata_t* audio = malloc(sizeof(struct audiodata_t));
    audio->tone_pos = 0;
    audio->tone_inc = 2 * 3.14159 * 1000 / 44100;
    SDL_AudioSpec* spec = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
    spec->freq = 44100;
    spec->format = AUDIO_U8;
    spec->channels = 1;
    spec->samples = 4096;
    spec->callback = *feed;
    spec->userdata = audio;
    return spec;
}

int sdl_initaudio() {
    SDL_AudioSpec* spec = sdl_initaudiospec();
    audio_device_id = SDL_OpenAudioDevice(NULL, 0, spec, NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    return (audio_device_id != 0);
}

void sdl_updateaudio(int enabled)  {
    if (enabled) {
        SDL_PauseAudioDevice(audio_device_id, 0);
    } else {
        SDL_PauseAudioDevice(audio_device_id, 1);
    }
}
