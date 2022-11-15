#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "cpu.h"
#include "debug.h"
#include "parser.h"
#include "sdl.h"
#include "snake.h"
#include "nes.h"
#include "listing.h"

int main(int argc, char **argv) {
  printf("hello nes\n");

  struct s_ines_parsed* ines = NULL;
  if (argc > 1) {
    ines = parse(argv[1]);
    if (!ines) return 1;
  } else {
   puts("no file to parse");
   return 2;
  }

  if (argc == 3 && strcmp(argv[2], "--listing") == 0) {
    listing(ines->prgrom, ines->ines->size_prgrom * 0x4000);
    return 0;
  }

  // snake();
  nes(*ines);
  free(ines);

  return 0;
}

