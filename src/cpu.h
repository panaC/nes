#ifndef CPU_H
#define CPU_H

#include <stdint.h>

union u_pc {

  uint16_t value;

  struct {

    uint8_t pcl;
    uint8_t pch;
  };
};

union u_p {

  uint8_t value;

  struct { 

    unsigned C: 1;
    unsigned Z: 1;
    unsigned D: 1;
    unsigned I: 1;

    unsigned B: 1;
    unsigned unused: 1;
    unsigned V: 1;
    unsigned N: 1;

  };
};

typedef struct s_registers {

  u_pc pc;

  uint8_t sp;

  u_p p;

  uint8_t a;

  uint8_t x;

  uint8_t y;

} t_registers;

typedef struct s_cpu {

  uint8_t* memory;

  struct s_registers registers;

} t_cpu;

typedef enum e_mode {
  immediate,
  zero_page,
  zero_page_x,
  absolute,
  absolute_x,
  absolute_y,
  indirect_x,
  indirect_y,
} t_e_mode;

#endif
