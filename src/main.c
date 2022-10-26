#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "cpu.h"
#include "test.h"
#include "debug.h"
#include "parser.h"
#include "sdl.h"
#include "snake.h"

int main(int argc, char **argv) {
  printf("hello nes\n");

  if (argc > 1) {
    parse(argv[1]);
  } else {
   puts("no file to parse");
  }

  snake_init();
  sdl_init(SNAKE_WIDTH, SNAKE_HEIGHT);
  cpu_init();

  SDL_Thread *thread;
  thread = SDL_CreateThread(cpu_run, "CPUThread", (void *)&__pause);
  if (!thread) {
    log_error("CPUThread ERROR");
    return 1;
  }

  snake();

  if (thread) {
    SDL_DetachThread(thread);
    thread = NULL;
  }

  sdl_quit();
  return 0;
}

