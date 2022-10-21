#include <stdio.h>
#include <stdlib.h> 
#include "bus.h"
#include "debug.h"
#include "sdl.h"
#include "snake.h"

#define debug(...) log_x(LOG_BUS, __VA_ARGS__)

// TODO:
// create a memory table with human readable name to print in bus debug

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
    debug("BUS: rand value %d", *value);
  }
}

static void bus_read_ff_rand(uint8_t *value, uint32_t addr) {
  if (addr == 0xff) {
    // rand
    *value = __lastkeycode;
    debug("BUS: last key %d", *value);
  }
}


static void bus_write_2xx_screen(uint32_t addr, uint8_t value) {
  if (addr >= 0x200 && addr < 0x600) {
    // fill a rect at address to the screen
    sdl_setPixelWithPitch(addr - 0x200, value ? 0xffffffff : 0);
    debug("SET PIXEL X=%d Y=%d", (addr-0x200) % 32, (addr-0x200) / 32);
  }
}

union u16 readbus16(uint32_t addr) {

  debug("READ16=0x%x", addr);
  assert(addr <= 0x736); // snake 0x736 bytes used

  union u16 v = {.lsb = readbus(addr), .msb = readbus(addr + 1)};
  return v;
}

uint8_t readbus(uint32_t addr) {

  debug("READ=0x%x VALUE=%d/%d/0x%x", addr, *__memory[addr], (int8_t)*__memory[addr], *__memory[addr]);
  assert(addr <= 0x737); // snake 0x736 bytes used // jump to 735 + READ16 736-737

  uint8_t value = *__memory[addr];
  bus_read_fe_rand(&value, addr);
  bus_read_ff_rand(&value, addr);

  return value;
}

uint8_t readbus_pc() {
  return readbus(__cpu_reg.pc + 1);
}

union u16 readbus16_pc(uint32_t addr) {
  return readbus16(__cpu_reg.pc + 1);
}

void writebus(uint32_t addr, uint8_t value) {

  debug("WRITE=0x%x VALUE=%d/%d", addr, value, (int8_t)value);
  assert(addr <= 0x736); // snake 0x736 bytes used
  bus_write_2xx_screen(addr, value);
  *__memory[addr] = value;
}