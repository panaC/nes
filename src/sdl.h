#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>

typedef enum {
  NO_EVENT = 0,
  QUIT_EVENT,
} t_sdl_action;

SDL_Window* sdl_init();
void sdl_quit();
SDL_Renderer *sdl_createRenderer();
void sdl_showRendering();
t_sdl_action sdl_processEvent();

// extern global variable
extern SDL_Window *__win;
extern SDL_Renderer *__renderer;

#endif