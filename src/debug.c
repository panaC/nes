#include <stdio.h>
#include "debug.h"
#include "cpu.h"

void print_register(t_registers *reg) {

  printf("pc=%x,sp=%x,p(C=%d,Z=%d,D=%d,I=%d,B=%d,V=%d,N=%d),a=%x,x=%x,y=%x", reg->pc, reg->sp, reg->p.C, reg->p.Z, reg->p.D, reg->p.I, reg->p.B, reg->p.V, reg->p.N, reg->a, reg->x, reg->y);
}