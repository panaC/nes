#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "cpu.h"
#include "test.h"
#include "debug.h"
#include "parser.h"
#include "sdl.h"
#include "snake.h"
#include "bus.h"

int global_verbose_level = 5;

int main(int argc, char **argv) {
  printf("hello nes\n");

  bus_init_memory();
  sdl_init();

  // run(&memory[0], MEM_SIZE, 0, NULL);
  // run_test(&memory[0]);
  // if (argc > 1) {
  //   parse(argv[1]);
  // } else 
  //  puts("no file to parse");

  // sdl_init();
  snake(&__memory[0]);

  bus_quit();
  sdl_quit();
  return 0;
}

