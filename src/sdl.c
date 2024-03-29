#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sdl.h"

#define debug(...) 0;

// global variable
SDL_Window *__win = NULL;
SDL_Renderer *__renderer = NULL;
SDL_Texture *__texture = NULL;

void sdl_init(int w, int h)
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
  __win = SDL_CreateWindow("hello nes", 10, 10, w, h, 0);
  if (!__win)
  {
    fprintf(stderr, "Couldn't set 640x640x8 video mode: %s\n",
            SDL_GetError());
    exit(1);
  }

  __renderer = SDL_CreateRenderer(__win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!__renderer)
  {
    fprintf(stderr, "Couldn't initialize renderer : %s", SDL_GetError());
    exit(1);
  }

  __texture = SDL_CreateTexture(__renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, w, h);
  if (!__texture)
  {
    fprintf(stderr, "Couldn't initialize texture: %s", SDL_GetError());
    exit(1);
  }

}
void sdl_showRendering(uint32_t *rawPixel, int w) {
  // assert(__renderer != NULL);

  SDL_UpdateTexture(__texture, NULL, rawPixel, w * sizeof(uint32_t));
  SDL_RenderClear(__renderer);
  SDL_RenderCopy(__renderer, __texture, NULL, NULL);
  SDL_RenderPresent(__renderer);
}

void sdl_quit()
{
  assert(__win != NULL);
  SDL_DestroyWindow(__win);
  SDL_Quit();
}

enum e_sdl_event sdl_processEvent() {
  SDL_Event event;
  enum e_sdl_event ret;

  SDL_PollEvent(&event);
  switch (event.type)
  {
  case SDL_QUIT:
    return SDL_QUIT_EVENT;

  case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_LEFT:
						return SDL_KD_MOVE_LEFT_EVENT;
					case SDLK_RIGHT:
						return SDL_KD_MOVE_RIGHT_EVENT;
					case SDLK_DOWN:
						return SDL_KD_MOVE_DOWN_EVENT;
					case SDLK_UP:
						return SDL_KD_MOVE_UP_EVENT;
					case SDLK_RETURN:
						return SDL_KD_RETURN_EVENT;
          case SDLK_SPACE:
            return SDL_KD_SPACE_EVENT;
				}
				break;

  default:
    // debug("UNKNOWN EVENT %d", event.type);
    return SDL_NO_EVENT;
  }

  return SDL_NO_EVENT;
}

void sdl_delay() {
  SDL_Delay(1000 / 60); // 60fps
}
