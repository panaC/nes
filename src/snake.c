#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "snake.h"
#include "stdint.h"
#include "utils.h"
#include "sdl.h"
#include "cpu.h"
#include "sdl.h"

#ifndef debug_content_SNAKE
#  define debug_content_SNAKE
#endif

#ifndef debug_content_SNAKE
#  define debug(...) 0;
#  define debug_start() 0;
#  define debug_content(...) 0;
#  define debug_end() 0;
#else
#  define debug_start() fprintf(stdout, "SNAKE: ");
#  define debug_content(...) fprintf(stdout, __VA_ARGS__);
#  define debug_end() fprintf(stdout, "\n");
#  define debug(...) debug_content_start();debug_content_content(__VA_ARGS__);debug_content_end();
#endif

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
static char lastkeycode = 0;
static uint32_t rawPixel[SNAKE_WIDTH * SNAKE_HEIGHT] = {0};

// extern global var
char __pause = 1; // pause at startup

static void setPixelWithPitch(uint32_t pos, uint32_t color) {
  uint32_t pixel;
  if (pos < (SNAKE_WIDTH * SNAKE_HEIGHT) / (SNAKE_PITCH * SNAKE_PITCH)) {
    for (int i = 0; i < SNAKE_PITCH; i++) {
      for (int j = 0; j < SNAKE_PITCH; j++) {
        pixel = (pos / (SNAKE_WIDTH / SNAKE_PITCH) * SNAKE_PITCH * SNAKE_WIDTH + i * SNAKE_WIDTH) + pos % (SNAKE_HEIGHT / SNAKE_PITCH) * SNAKE_PITCH + j;
        rawPixel[pixel] = color;
      }
    }
  }
}

uint8_t bus_read_fe_rand(uint8_t value, uint32_t addr) {
  if (addr == 0xfe) {
    value =  rand() % 256;
    debug_content("BUS: rand value %d | ", value);
  }
  return value;
}

uint8_t bus_read_ff_rand(uint8_t value, uint32_t addr) {
  if (addr == 0xff) {
    value = lastkeycode;
    debug_content("BUS: last key %d | ", value);
  }
  return value;
}

uint8_t bus_write_2xx_screen(uint8_t value, uint32_t addr) {
  if (addr >= 0x200 && addr < 0x600) {
    // fill a rect at address to the screen
    setPixelWithPitch(addr - 0x200, value ? 0xffffffff : 0);
    debug_content("SET PIXEL X=%d Y=%d | ", (addr-0x200) % 32, (addr-0x200) / 32);
  }
  return value;
}

uint8_t mem[0x10000] = {0};
uint8_t snake_readbus(uint32_t addr) {

  uint8_t value = mem[addr];
  value = bus_read_fe_rand(value, addr);
  value = bus_read_ff_rand(value, addr);

  if (addr >= START && addr < START + sizeof(rom))
    value = rom[addr - START];

  if (addr == 0xfffd)
    value = 0x06;
  if (addr == 0xfffc)
    value = 0x00;

  debug_content("r[%04x]=%02x | ", addr, value);

  return value;
}

void snake_writebus(uint32_t addr, uint8_t value) {

  value = bus_write_2xx_screen(value, addr);
  mem[addr] = value;

  debug_content("w[%04x]=%02x | ", addr, value);
}

void snake_init() {

  cpu_readbus = &snake_readbus;
  cpu_writebus = &snake_writebus;
}

void snake() {

  // memory map
  // 0x000 -> 0x0ff : variable
  // 0x100 -> 0x1ff : stack
  // 0x200 -> 0x5ff : screen 32 * 32
  // 0x600 -> sizeof(rom) : rom
  // memory_size = 0x1000 : 16 * 256 : 4kB

  sdl_init(SNAKE_WIDTH, SNAKE_HEIGHT);
  snake_init();
  cpu_init();

  struct timespec t1 = {0};
  struct timespec t2 = {0};
  uint64_t time_elasped = 0;
  uint64_t freq_delay = 1.0e9 / (1000 * 10);

  struct timespec sleep = {
    .tv_sec = 0,
    .tv_nsec = freq_delay,
  };

  int pause = 0;
  int cycles;
  uint16_t pc;

  for (;;) {

    debug_start();

    switch (sdl_processEvent())
    {
    case SDL_QUIT_EVENT:
      goto end;
    case SDL_KD_MOVE_DOWN_EVENT:
      lastkeycode = 0x73;
      debug_content("MOVE DOWN");
      break;
    case SDL_KD_MOVE_UP_EVENT:
      lastkeycode = 0x77;
      debug_content("MOVE UP");
      break;
    case SDL_KD_MOVE_LEFT_EVENT:
      lastkeycode = 0x61;
      debug_content("MOVE LEFT");
      break;
    case SDL_KD_MOVE_RIGHT_EVENT:
      lastkeycode = 0x64;
      debug_content("MOVE RIGHT");
      break;
    case SDL_KD_SPACE_EVENT:
      pause = !pause;
      debug_content("PAUSE");
      break;

    default:
      break;
    }

    if (pause) {
      SDL_Delay(100);
      continue;
    }

    clock_gettime(CLOCK_REALTIME, &t1);

    cycles = cpu_exec(&pc);
    if (cycles == -1)
      break;

    // clock_gettime(CLOCK_MONOTONIC, &t2);
    clock_gettime(CLOCK_REALTIME, &t2);
    
    time_elasped = t2.tv_nsec - t1.tv_nsec < 0 ? (1.0e9 + (t2.tv_nsec - t1.tv_nsec)) : (t2.tv_nsec - t1.tv_nsec);
    debug_content("TIME=%ld ", time_elasped);
    assert(freq_delay - time_elasped > 0);
    sleep.tv_nsec = freq_delay - time_elasped;
    nanosleep(&sleep, NULL);

    if (pc == 0x638) {
      sdl_showRendering(rawPixel, SNAKE_WIDTH);
      debug_content("SHOW RENDERING");
    }

    debug_end();
  }

end:
  sdl_quit();
}
