#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include "log.h"

void sdl_init(int w, int h);
void sdl_quit();
void sdl_showRendering(uint32_t *__rawPixel, int w);

// extern global variable
extern SDL_Window *__win;
extern SDL_Renderer *__renderer;
extern SDL_Texture *__texture;

#endif