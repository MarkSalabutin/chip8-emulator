#include "chip8.h"

extern bool draw_flag;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./chip8 /path/to/game\n");
        exit(2);
    }
    
    SDL_Event event;
    bool running = true;
    
    chip8_initialize();
    chip8_loadgame(argv[1]);
    
    while (running) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || SDL_GetKeyboardState(NULL)[SDL_SCANCODE_ESCAPE]) {
                running = false;
            }
        }
        
        chip8_emulatecycle();
        chip8_tick();
        
        if (draw_flag == true) {
            sdl_drawgraphics();
            draw_flag = false;
        }
        
        
        SDL_Delay(2);
    }
    
    sdl_cleanup();
    exit(EXIT_SUCCESS);

    return 0;
}
