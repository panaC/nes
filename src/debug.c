#include <stdio.h>
#include "debug.h"
#include "cpu.h"

void print_register(t_registers *reg) {

  log_x(LOG_REGISTER, "pc=%02x,sp=%02x,p(C=%d,Z=%d,D=%d,I=%d,B=%d,V=%d,N=%d),a=%02x,x=%02x,y=%02x", reg->pc, reg->sp, reg->p.C, reg->p.Z, reg->p.D, reg->p.I, reg->p.B, reg->p.V, reg->p.N, reg->a & 0xff, reg->x & 0xff, reg->y & 0xff);
}