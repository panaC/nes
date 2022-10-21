#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stdint.h>
#include "bus.h"

#ifndef CPU_FREQ
#define CPU_FREQ 1
#endif

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

  // need to be int16 for correct overflow handling
  int16_t a;
  int16_t x;
  int16_t y;

} t_registers;

void irq(t_registers *reg, t_mem *memory);
int cpu_run();
int cpu_exec(t_mem *memory, t_registers *reg);
void run(t_mem *memory, size_t size, t_registers *reg);

// global extern var
extern t_registers __cpu_reg;

#endif
