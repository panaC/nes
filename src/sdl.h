#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>

SDL_Window* sdl_init();
void sdl_quit();
SDL_Renderer *sdl_createRenderer();
void sdl_showRendering();

// extern global variable
extern SDL_Window *__win;
extern SDL_Renderer *__renderer;

#endif