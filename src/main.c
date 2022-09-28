#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "cpu.h"
#include "test.h"
#include "debug.h"
#include "parser.h"
#include "sdl.h"

int global_verbose_level = 5;

int main(int argc, char **argv) {
  printf("hello nes\n");

  uint8_t* rawmem = (uint8_t*) malloc(MEM_SIZE);
  bzero(rawmem, MEM_SIZE);
  t_mem memory[MEM_SIZE];
  for (int i = 0; i < MEM_SIZE; i++) {
    memory[i] = rawmem + i;
  }

  // run(&memory[0], MEM_SIZE, 0, NULL);
  // run_test(&memory[0]);
  // if (argc > 1) {
  //   parse(argv[1]);
  // } else 
  //  puts("no file to parse");

  sdl_init();

  free(rawmem);
  return 0;
}

