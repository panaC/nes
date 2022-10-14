#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include "log.h"

#define HEIGHT 640
#define WIDTH 640
#define PITCH 20

typedef enum {
  NO_EVENT = 0,
  QUIT_EVENT,
  MOVE_UP,
  MOVE_DOWN,
  MOVE_LEFT,
  MOVE_RIGHT,
  PAUSE
} t_sdl_action;

void sdl_init();
void sdl_quit();
void sdl_showRendering();
void sdl_setPixelWithPitch(uint32_t pos, uint32_t color);
t_sdl_action sdl_processEvent();

// extern global variable
extern SDL_Window *__win;
extern SDL_Renderer *__renderer;
extern SDL_Texture *__texture;
extern uint32_t __rawPixel[WIDTH * HEIGHT];

#endif