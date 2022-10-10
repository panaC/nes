#include <stdio.h>
#include <stdlib.h> 
#include "bus.h"
#include "debug.h"
#include "sdl.h"

// TODO:
// create a memory table with human readable name to print in bus debug

static int ____show_log = 1;
t_mem __memory[MEM_SIZE] = {NULL};

void bus_init_memory() {
  uint8_t* rawmem = (uint8_t*) malloc(MEM_SIZE);
  bzero(rawmem, MEM_SIZE);
  for (int i = 0; i < MEM_SIZE; i++) {
    __memory[i] = rawmem + i;
  }
}

void bus_quit(void){
}

static void bus_read_fe_rand(uint8_t *value, uint32_t addr) {
  if (addr == 0xfe) {
    // rand
    *value = rand() % 256;
    printf("BUS: rand value %d\n", *value);
  }
}

static void bus_write_2xx_screen(uint32_t addr, uint8_t value) {
  if (addr >= 0x200 && addr < 0x600) {
    // fill a rect at address to the screen

    SDL_SetRenderDrawColor(__renderer, 0xff, 0xff, 0xff, 0xff); // white
    SDL_Rect rect = {.x = ((addr - 0x200) % 32) * 20, .y = ((addr - 0x200) / 32) * 20, .w = 20, .h = 20};
    SDL_RenderFillRect(__renderer, &rect);
    VB3(printf("SDL: Fill one rect at addr=0x%x, x=%d, y=%d", addr, rect.x, rect.y));
  }
}

union u16 readbus16(t_mem *memory, uint32_t addr) {

  memory = NULL; // memory not used anymore -> see global variable __memory

  VB3(printf("BUS: READ16=0x%x", addr));
  assert(addr <= 0x736); // snake 0x736 bytes used

  ____show_log = 0;
  union u16 v = {.lsb = readbus(__memory, addr), .msb = readbus(__memory, addr + 1)};
  ____show_log = 1;
  return v;
}

uint8_t readbus(t_mem *memory, uint32_t addr) {

  memory = NULL; // memory not used anymore -> see global variable __memory

  if (____show_log)
    VB3(printf("BUS: READ=0x%x VALUE=%d/%d/0x%x", addr, *__memory[addr], (int8_t)*__memory[addr], *__memory[addr]));
  assert(addr <= 0x736); // snake 0x736 bytes used

  uint8_t value = *__memory[addr];
  bus_read_fe_rand(&value, addr);

  return value;
}
void writebus(t_mem *memory, uint32_t addr, uint8_t value) {

  memory = NULL; // memory not used anymore -> see global variable __memory

  VB3(printf("BUS: WRITE=%x VALUE=%d/%d", addr, value, (int8_t)value));
  assert(addr <= 0x736); // snake 0x736 bytes used
  bus_write_2xx_screen(addr, value);
  *__memory[addr] = value;
}