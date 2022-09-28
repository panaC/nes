#include <stdio.h>
#include "bus.h"
#include "debug.h"


// TODO:
// create a memory table with human readable name to print in bus debug

union u16 readbus16(t_mem *memory, uint32_t addr) {
  VB3(printf("BUS: READ16=0x%x", addr));
  return (union u16){.lsb = MEM_GET(memory, addr), .msb = MEM_GET(memory, addr + 1)};
}

uint8_t readbus(t_mem *memory, uint32_t addr) {
  VB3(printf("BUS: READ=0x%x VALUE=%d/%d/0x%x", addr, MEM_GET(memory, addr), (int8_t)MEM_GET(memory, addr), MEM_GET(memory, addr)));
  return MEM_GET(memory, addr);
}
void writebus(t_mem *memory, uint32_t addr, uint8_t value) {
  VB3(printf("BUS: WRITE=%x VALUE=%d/%d", addr, value, (int8_t)value));
  MEM_SET(memory, addr, value);
}