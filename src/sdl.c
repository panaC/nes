#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

void sdl_init()
{

  SDL_Window *win;

  /* Initialize the SDL library */
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr,
            "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  /*
   * Initialize the display in a 640x480 8-bit palettized mode,
   * requesting a software surface
   */
  win = SDL_CreateWindow("hello nes", 10, 10, 640, 480, SDL_WINDOW_RESIZABLE);
  if (!win)
  {
    fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n",
            SDL_GetError());
    exit(1);
  }

  int isquit = 0;
  SDL_Event event;
  while (!isquit)
  {
    if (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        isquit = 1;
      }
    }
  }

  SDL_DestroyWindow(win);

  SDL_Quit();
}