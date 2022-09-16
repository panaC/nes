#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stdint.h>

union u16 {

  uint16_t value;

  struct {

    uint8_t lsb;
    uint8_t msb;
  };
};

union u_p {

  uint8_t value;

  struct { 

    unsigned C: 1; // carry
    unsigned Z: 1; // Zero
    unsigned D: 1; // Decimal
    unsigned I: 1; // Interruption
    unsigned B: 1; // break
    unsigned unused: 1;
    unsigned V: 1; // overflow
    unsigned N: 1; // negative

  };
};

typedef struct s_registers {

  uint16_t pc;

  uint8_t sp;

  union u_p p;

  int8_t a; // not unsigned 

  uint8_t x;

  uint8_t y;

} t_registers;

// typedef struct s_cpu {

//   uint8_t* memory;

//   struct s_registers registers;

// } t_cpu;

// is it usefull ? 
typedef enum e_mode {
  zero_page,
  zero_page_x,
  zero_page_y,
  absolute,
  absolute_x,
  absolute_y,
  indirect_x,
  indirect_y,
} t_e_mode;

#define MEM_SIZE 1024 // 64KB // 2 ^16 // 65536
typedef uint8_t* t_mem;

void irq(t_registers *reg, t_mem *memory);
void run(t_mem *memory, size_t size, t_registers *reg);

#endif
