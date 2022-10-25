#include <stdio.h>
#include <stdlib.h> 
#include "bus.h"
#include "debug.h"
#include "sdl.h"
#include "snake.h"

#define debug(...) log_x(LOG_BUS, __VA_ARGS__)

// extern global
t_mem __memory[MEM_SIZE] = {0};

#define EVENT_BUS_FN_SIZE 16
// global
readwritefn readFnEventBus[EVENT_BUS_FN_SIZE] = {NULL};
size_t readEventBusSize = 0;
readwritefn writeFnEventBus[EVENT_BUS_FN_SIZE] = {NULL};
size_t writeEventBusSize = 0;

void bus_init() {
  uint8_t *rawmem = (uint8_t *)malloc(MEM_SIZE);
  bzero(rawmem, MEM_SIZE);
  for (int i = 0; i < MEM_SIZE; i++) {
    __memory[i] = rawmem + i;
  }
}

readwritefn bus_read_on(readwritefn fn) {
  if (!fn) return NULL;
  for (int i = 0; i < readEventBusSize; i++) {
    if (fn == readFnEventBus[i]) return NULL;
  }
  readFnEventBus[readEventBusSize] = fn;
  readEventBusSize++;

  return fn;
}

readwritefn bus_write_on(readwritefn fn) {
  if (!fn) return NULL;
  for (int i = 0; i < readEventBusSize; i++) {
    if (fn == readFnEventBus[i]) return NULL;
  }
  writeFnEventBus[writeEventBusSize] = fn;
  writeEventBusSize++;

  return fn;
}

union u16 readbus16(uint32_t addr) {

  debug("READ16=0x%x", addr);

  union u16 v = {.lsb = readbus(addr), .msb = readbus(addr + 1)};
  return v;
}

uint8_t readbus(uint32_t addr) {

  debug("READ=0x%x VALUE=%d/%d/0x%x", addr, *__memory[addr], (int8_t)*__memory[addr], *__memory[addr]);

  uint8_t value = *__memory[addr];
  for (int i = 0; i < readEventBusSize; i++) {
    value = readFnEventBus[i](value, addr);
  }

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

  for (int i = 0; i < writeEventBusSize; i++) {
    value = writeFnEventBus[i](value, addr);
  }

  *__memory[addr] = value;
}