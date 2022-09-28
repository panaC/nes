#ifndef BUS_H
#define BUS_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

union u16 {

  uint16_t value;

  struct {

    uint8_t lsb;
    uint8_t msb;
  };
};

#define MEM_SIZE 65536 // 64KB // 2 ^16 // 65536
typedef uint8_t* t_mem;

#define MEM_SET(mem, x, value) assert(x < MEM_SIZE); *mem[x] = value
#define MEM_PTR(mem, x) (assert((int32_t)x < MEM_SIZE), mem[x])
#define MEM_GET(mem, x) *MEM_PTR(mem, x)

// declare to function read and write dans la mémoire
// et une 3eme pour obtenir le ptr

uint8_t readbus(t_mem *memory, uint32_t addr);
union u16 readbus16(t_mem *memory, uint32_t addr);
void writebus(t_mem *memory, uint32_t addr, uint8_t value);

#endif