#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sdl.h"

// global variable
SDL_Window *__win = NULL;
SDL_Renderer *__renderer = NULL;

SDL_Window *sdl_init()
{

  /* Initialize the SDL library */
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr,
            "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  /*
   * Initialize the display in a 640x640 8-bit palettized mode,
   * requesting a software surface
   *
   * 32 * 20 pixels pitch
   */
  __win = SDL_CreateWindow("hello nes", 10, 10, 640, 640, 0);
  if (!__win)
  {
    fprintf(stderr, "Couldn't set 640x640x8 video mode: %s\n",
            SDL_GetError());
    exit(1);
  }

  return __win;
}

SDL_Renderer *sdl_createRenderer()
{

  assert(__win != NULL);
  __renderer = SDL_CreateRenderer(__win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!__renderer)
  {
    fprintf(stderr, "Couldn't initialize renderer : %s", SDL_GetError());
    exit(1);
  }

  SDL_SetRenderDrawColor(__renderer, 0, 0, 0, 255);
  SDL_RenderClear(__renderer);
  return __renderer;
}

void sdl_showRendering() {
  assert(__renderer != NULL);
  SDL_RenderPresent(__renderer);
}

t_sdl_action sdl_processEvent() {
  SDL_Event event;
  t_sdl_action ret;

  SDL_PollEvent(&event);
  switch (event.type)
  {
  case SDL_QUIT:
    return QUIT_EVENT;
    break;

  default:
    return NO_EVENT;
  }
}

void sdl_quit()
{
  assert(__win != NULL);
  SDL_DestroyWindow(__win);
  SDL_Quit();
}
