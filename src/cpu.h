
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

struct s_registers {

  u_pc pc;

  uint8_t sp;

  u_p p;

  uint8_t x;

  uint8_t y;

};
