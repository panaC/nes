#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>

enum e_sdl_event {
  SDL_NO_EVENT,
  SDL_QUIT_EVENT,
  SDL_KD_MOVE_LEFT_EVENT,
  SDL_KD_MOVE_RIGHT_EVENT,
  SDL_KD_MOVE_DOWN_EVENT,
  SDL_KD_MOVE_UP_EVENT,
  SDL_KD_RETURN_EVENT,
  SDL_KD_SPACE_EVENT,
};

void sdl_init(int w, int h);
void sdl_quit();
void sdl_showRendering(uint32_t *__rawPixel, int w);
enum e_sdl_event sdl_processEvent();
void sdl_delay();

// extern global variable
extern SDL_Window *__win;
extern SDL_Renderer *__renderer;
extern SDL_Texture *__texture;

#endif