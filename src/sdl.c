#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sdl.h"

#define debug(...) log_x(LOG_SDL, __VA_ARGS__)

// global variable
SDL_Window *__win = NULL;
SDL_Renderer *__renderer = NULL;
SDL_Texture *__texture = NULL;
uint32_t __rawPixel[WIDTH * HEIGHT] = {0};

void sdl_init()
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
  __win = SDL_CreateWindow("hello nes", 10, 10, WIDTH, HEIGHT, 0);
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

  __texture = SDL_CreateTexture(__renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
  if (!__texture)
  {
    fprintf(stderr, "Couldn't initialize texture: %s", SDL_GetError());
    exit(1);
  }

}

void sdl_setPixelWithPitch(uint32_t pos, uint32_t color) {
  uint32_t pixel;
  if (pos < (WIDTH * HEIGHT) / (PITCH * PITCH)) {
    for (int i = 0; i < PITCH; i++) {
      for (int j = 0; j < PITCH; j++) {
        pixel = (pos / (WIDTH / PITCH) * PITCH * WIDTH + i * WIDTH) + pos % (HEIGHT / PITCH) * PITCH + j;
        __rawPixel[pixel] = color;
      }
    }
  }
}

void sdl_showRendering() {
  // assert(__renderer != NULL);

  SDL_UpdateTexture(__texture, NULL, __rawPixel, WIDTH * sizeof(uint32_t));
  SDL_RenderClear(__renderer);
  SDL_RenderCopy(__renderer, __texture, NULL, NULL);
  SDL_RenderPresent(__renderer);
}

t_sdl_action sdl_processEvent() {
  SDL_Event event;
  t_sdl_action ret;

  SDL_PollEvent(&event);
  switch (event.type)
  {
  case SDL_QUIT:
    debug("QUIT EVENT");
    return QUIT_EVENT;
    break;

  default:
    // debug("UNKNOWN EVENT %d", event.type);
    return NO_EVENT;
  }
}

void sdl_quit()
{
  assert(__win != NULL);
  SDL_DestroyWindow(__win);
  SDL_Quit();
}
