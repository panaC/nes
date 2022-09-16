#ifndef BUS_H
#define BUS_H

#include <assert.h>
#include <stddef.h>


#define MEM_SIZE 1024 // 64KB // 2 ^16 // 65536
typedef uint8_t* t_mem;

#define MEM_SET(mem, x, value) assert(x < MEM_SIZE); *mem[x] = value;
#define MEM_PTR(mem, x) (assert((int32_t)x < MEM_SIZE), mem[x]);
#define MEM_GET(mem, x) *MEM_PTR(mem, x);

#endif