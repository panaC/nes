#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stdint.h>
#include "bus.h"

#ifndef CPU_FREQ
#define CPU_FREQ (1000 * 1000 * 1) // 10Mhz snake
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

enum e_addressMode {
  ACCUMULATOR,
  IMPLIED,
  IMMEDIATE,
  RELATIVE,
  ZEROPAGE,
  ZEROPAGEX,
  ZEROPAGEY,
  ABSOLUTE,
  ABSOLUTEX,
  ABSOLUTEY,
  INDIRECT,
  INDIRECTX,
  INDIRECTY,
};

typedef int16_t (*opfn)(enum e_addressMode mode, union u16 uarg);
typedef void (*spfn)(int16_t value);

struct instruction {
  char *str;
  uint8_t code;
  enum e_addressMode mode;
  int size;
  int cycles;
  int crossed;
  opfn fn;
  spfn end;
};

void cpu_init();
void cpu_irq();
int cpu_run();
int cpu_exec(t_mem *memory, t_registers *reg);
void run(t_mem *memory, size_t size, t_registers *reg);

// global extern var
extern t_registers __cpu_reg;

#endif
