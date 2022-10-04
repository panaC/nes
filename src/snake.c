#include <stdio.h>
#include "snake.h"
#include "stdint.h"
#include "utils.h"
#include "sdl.h"
#include "debug.h"

uint8_t rom[] = {
  0x20, 0x06, 0x06, 0x20, 0x38, 0x06, 0x20, 0x0d, 0x06, 0x20, 0x2a, 0x06, 0x60, 0xa9, 0x02, 0x85,
  0x02, 0xa9, 0x04, 0x85, 0x03, 0xa9, 0x11, 0x85, 0x10, 0xa9, 0x10, 0x85, 0x12, 0xa9, 0x0f, 0x85,
  0x14, 0xa9, 0x04, 0x85, 0x11, 0x85, 0x13, 0x85, 0x15, 0x60, 0xa5, 0xfe, 0x85, 0x00, 0xa5, 0xfe,
  0x29, 0x03, 0x18, 0x69, 0x02, 0x85, 0x01, 0x60, 0x20, 0x4d, 0x06, 0x20, 0x8d, 0x06, 0x20, 0xc3,
  0x06, 0x20, 0x19, 0x07, 0x20, 0x20, 0x07, 0x20, 0x2d, 0x07, 0x4c, 0x38, 0x06, 0xa5, 0xff, 0xc9,
  0x77, 0xf0, 0x0d, 0xc9, 0x64, 0xf0, 0x14, 0xc9, 0x73, 0xf0, 0x1b, 0xc9, 0x61, 0xf0, 0x22, 0x60,
  0xa9, 0x04, 0x24, 0x02, 0xd0, 0x26, 0xa9, 0x01, 0x85, 0x02, 0x60, 0xa9, 0x08, 0x24, 0x02, 0xd0,
  0x1b, 0xa9, 0x02, 0x85, 0x02, 0x60, 0xa9, 0x01, 0x24, 0x02, 0xd0, 0x10, 0xa9, 0x04, 0x85, 0x02,
  0x60, 0xa9, 0x02, 0x24, 0x02, 0xd0, 0x05, 0xa9, 0x08, 0x85, 0x02, 0x60, 0x60, 0x20, 0x94, 0x06,
  0x20, 0xa8, 0x06, 0x60, 0xa5, 0x00, 0xc5, 0x10, 0xd0, 0x0d, 0xa5, 0x01, 0xc5, 0x11, 0xd0, 0x07,
  0xe6, 0x03, 0xe6, 0x03, 0x20, 0x2a, 0x06, 0x60, 0xa2, 0x02, 0xb5, 0x10, 0xc5, 0x10, 0xd0, 0x06,
  0xb5, 0x11, 0xc5, 0x11, 0xf0, 0x09, 0xe8, 0xe8, 0xe4, 0x03, 0xf0, 0x06, 0x4c, 0xaa, 0x06, 0x4c,
  0x35, 0x07, 0x60, 0xa6, 0x03, 0xca, 0x8a, 0xb5, 0x10, 0x95, 0x12, 0xca, 0x10, 0xf9, 0xa5, 0x02,
  0x4a, 0xb0, 0x09, 0x4a, 0xb0, 0x19, 0x4a, 0xb0, 0x1f, 0x4a, 0xb0, 0x2f, 0xa5, 0x10, 0x38, 0xe9,
  0x20, 0x85, 0x10, 0x90, 0x01, 0x60, 0xc6, 0x11, 0xa9, 0x01, 0xc5, 0x11, 0xf0, 0x28, 0x60, 0xe6,
  0x10, 0xa9, 0x1f, 0x24, 0x10, 0xf0, 0x1f, 0x60, 0xa5, 0x10, 0x18, 0x69, 0x20, 0x85, 0x10, 0xb0,
  0x01, 0x60, 0xe6, 0x11, 0xa9, 0x06, 0xc5, 0x11, 0xf0, 0x0c, 0x60, 0xc6, 0x10, 0xa5, 0x10, 0x29,
  0x1f, 0xc9, 0x1f, 0xf0, 0x01, 0x60, 0x4c, 0x35, 0x07, 0xa0, 0x00, 0xa5, 0xfe, 0x91, 0x00, 0x60,
  0xa6, 0x03, 0xa9, 0x00, 0x81, 0x10, 0xa2, 0x00, 0xa9, 0x01, 0x81, 0x10, 0x60, 0xa2, 0x00, 0xea,
  0xea, 0xca, 0xd0, 0xfb, 0x60, 0x00, // game-over
};

#define START 0x600

static int quit = 0;

static int CPUThread(void *data) {
  t_registers reg = {
      .pc = START,
      .sp = 0,
      .p = 0,
      .a = 0,
      .x = 0,
      .y = 0};

  int debug = 0;
  int brk = 0x0724;
  while (1) {
    if (reg.pc == brk) debug = 1;
    if (debug) {
      int c = getchar();
      if (c == 'p') {
        hexdumpSnake(*(__memory + 0x200), 1024);
        while(getchar() != '\n');
        continue;
      } else if (c == 'r') {
        debug = 0;
        while(getchar() != '\n');
        continue;
      } else if (c == 's') {
        sdl_showRendering();
        while(getchar() != '\n');
        continue;
      }
      // lf 10
    }

    quit = exec(__memory, &reg);
  }
}

void snake() {

  // memory map
  // 0x000 -> 0x0ff : variable
  // 0x100 -> 0x1ff : stack
  // 0x200 -> 0x5ff : screen 32 * 32
  // 0x600 -> sizeof(rom) : rom
  // memory_size = 0x1000 : 16 * 256 : 4kB


  for(int i = 0; i < sizeof(rom); i++) {
    __memory[START + i] = rom + i;
  }

  hexdump(*(__memory + START), 320);

// 1/ launch SDL
// 2/ launch CPU
// 3/ get keyboard event
// 4/ display buffer 0x200 -> 0x5ff -> 32 * 32
//        grid of 640 * 640 -> *20 -> pitch 20pixels



  int quit = 0;
  SDL_Event event;
  SDL_Thread *thread;

  thread = SDL_CreateThread(CPUThread, "CPUThread", (void *)NULL);
  if (!thread) {
    printf("CPUThread ERROR");
  }

  while (quit != -1)
  {
    if (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        if (thread)
        {
          SDL_DetachThread(thread);
          thread = NULL;
        }
        quit = -1;
      }
    }
  }

  hexdumpSnake(*(__memory + 0x200), 1024);

}