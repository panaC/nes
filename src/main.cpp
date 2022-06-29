
#include <iostream>
#include <stdio.h>

#include <fmt/core.h>

#include "cpu.h"

int main(int argc, char **argv) {
  
  union u_p p;

  p.value = 0;
  p.N = (0b00000000 & 0b10000000) >> 7;
  fmt::print("Hello World {:#04x}\n", p.value);

  p.N = (0b10000000 & 0b10000000) >> 7;
  fmt::print("Hello World {:#04x}\n", p.value);

  return 0;
}
