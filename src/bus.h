#ifndef BUS_H
#define BUS_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "log.h"

union u16 {

  uint16_t value;

  struct {

    uint8_t lsb;
    uint8_t msb;
  };
};

#define MEM_SIZE 65536 // 64KB // 2 ^16 // 65536

typedef uint8_t *t_mem;
typedef uint8_t (*readwritefn)(uint8_t value, uint32_t addr);

void bus_init_memory(void);
uint8_t readbus(uint32_t addr);
union u16 readbus16(uint32_t addr);
uint8_t readbus_pc();
union u16 readbus16_pc();
void writebus(uint32_t addr, uint8_t value);

readwritefn bus_read_on(readwritefn fn);
readwritefn bus_write_on(readwritefn fn);

// extern global variable
extern t_mem __memory[MEM_SIZE];

#endif