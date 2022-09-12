#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

int main(int argc, char **argv) {
  printf("hello nes\n");

  uint8_t* rawmem = (uint8_t*) malloc(MEM_SIZE);
  t_mem memory[MEM_SIZE];
  for (int i = 0; i < MEM_SIZE; i++) {
    memory[i] = rawmem + i;
  }

  run(&memory[0], MEM_SIZE, 0);

  free(rawmem);
  return 0;
}

