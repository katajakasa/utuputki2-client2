#include "framework.h"

#include <SDL2/SDL.h>
#include <stdio.h>

int framework_init() {
	fprintf(stderr, "Initializing framework subsystem ...\n");

    // Init SDL2
    if(SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_AUDIO)) {
        fprintf(stderr, "SDL2 Initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    // Print SDL2 information
    SDL_version sdl_linked;
    SDL_GetVersion(&sdl_linked);
    fprintf(stderr, " * Found SDL v%d.%d.%d\n", sdl_linked.major, sdl_linked.minor, sdl_linked.patch);
    fprintf(stderr, " * Running on platform: %s\n", SDL_GetPlatform());
    return 0;
}

void framework_close() {
	fprintf(stderr, "Closing framework subsystem.\n");
	SDL_Quit();
}
