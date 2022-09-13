#include <stdio.h>
#include "cpu.h"
#include "debug.h"

// todo:  global variable
// handle State Machine with cycle
uint32_t cycle = 0; 
int adc_opcode(t_registers *reg, t_mem *memory);


// TODO : 
// setup a memory checker
// vérfier que l'addresse demandé en mémoire existe bien. Et print une error
// Faire ça avec une macro

void run(t_mem *memory, size_t size, t_registers *reg) {

  VB0(printf("RUN 6502 with a memory of %zu octets", size));

  // pc = start;
  // parse memory with opcode
  // if (reg == NULL) {
  //   t_registers r = {
  //       .pc = start,
  //       .sp = 0,
  //       .p = 0,
  //       .a = 0,
  //       .x = 0,
  //       .y = 0};
  //   reg = &r;
  // }
  // *memory[0] = 0x42;
  // *memory[1] = 0x42;
  // *memory[2] = 0x42;
  // *memory[3] = 0x42;
  // *memory[4] = 0x42;

  // printf("%d\n", *memory);

  while(1) {
    int res = 0;
    uint8_t op = *memory[reg->pc];
    if (op == 0) {
      // break;
      return;
    }
    res += adc_opcode(reg, memory);
    if (res) {
      // found
    } else {
      VB2(printf("OP=%x not found", op));
      reg->pc++;
    }

    VB4(print_register(reg));

  }


}

void check_processor_status(int32_t lastValue, t_registers *reg)
{
  if (lastValue > 127 || lastValue < -128)
    reg->p.C = 1; // overflow or underflow
  if (reg->a == 0)
    reg->p.Z = 1; // Zero
  if (reg->a < 0)
    reg->p.N = 1; // Negatif
  if (lastValue > reg->a)
    reg->p.V = 1; // overflow
}

uint8_t addressMode(t_e_mode mode, union u16 arg, t_registers *reg, t_mem *memory) {

  VB4(printf("addressMode arg(LSB=%x,MSB=%x,v=%x)", arg.lsb, arg.msb, arg.value));

  switch (mode)
  {
  case zero_page:
    return *memory[arg.lsb];
  case zero_page_x:
    return *memory[(arg.lsb + reg->x) % 256];
  case zero_page_y:
    return *memory[(arg.lsb + reg->y) % 256];
  case absolute:
    return *memory[arg.lsb];
  case absolute_x:
    return *memory[arg.value + reg->x];
  case absolute_y:
    return *memory[arg.value + reg->y];
  case indirect_x:
    return *memory[*memory[(arg.lsb + reg->x) % 256] + *memory[(arg.lsb + reg->x + 1) % 256] * 256];
  case indirect_y:
    return *memory[*memory[arg.lsb] + *memory[(arg.lsb + 1) % 256] * 256 + reg->y];
  }
  return 0;
}

int adc_opcode(t_registers *reg, t_mem *memory) {
  int16_t lastValue;


  uint8_t op = *memory[reg->pc];
  union u16 addr = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};

  switch (op)
  {
  case 0x69: // immediate
    VB2(printf("adc opcode immediate"));
    lastValue = reg->a + addr.lsb + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 2;
    cycle += 2;

    break;
  case 0x65:
    VB2(printf("adc opcode zeropage"));
    lastValue = reg->a + addressMode(zero_page, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 2;
    cycle += 3;

    break;
  case 0x75: 
    VB2(printf("adc opcode zeropagex"));
    lastValue = reg->a + addressMode(zero_page_x, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 2;
    cycle += 4;

    break;
  case 0x6d: 
    VB2(printf("adc opcode absolute"));
    lastValue = reg->a + addressMode(absolute, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 3;
    cycle += 4;

    break;
  case 0x7d: 
    VB2(printf("adc opcode absolutex"));
    lastValue = reg->a + addressMode(absolute_x, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 3;
    if ((addr.value & 0x00ff) + reg->x > 0xff)
      cycle += 5;
    else
      cycle += 4;
    // +1 if page crossed // how to do this ?
    // page crossed : 
    // $3000 and $30FF -> valeur de x entre 0 et ff page de page crossed
    // 30ff -> x > 0 = page crossed
    // 0001 0000  0000 0000
    // 0000 0000  1000 0000
    // 0001 0000  1000 0000
    // masque sur les bits de poid fort
    // if carry sur l'addition alors page+1

    break;
  
  case 0x79: 
    lastValue = reg->a + addressMode(absolute_y, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 3;
    if ((addr.value & 0x00ff) + reg->x > 0xff)
      cycle += 5;
    else
      cycle += 4;

    break;

   case 0x61: 
    lastValue = reg->a + addressMode(indirect_x, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 2;
    cycle += 6;

    break; 
   case 0x71: 
    lastValue = reg->a + addressMode(indirect_y, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    // how to do a page crossing detection on indirect_y
    // val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256) * 256 + Y)
    // perhaps before Y !?
    // not so easy the address generation is done in function addressMode
    // TODO implement this!!

    reg->pc += 2;
    cycle += 5;
    // if (addr & 0x00ff + reg.x > 0xff)
    //   reg->pc += 6;
    // else
    //   reg->pc += 5;

    break; 
    default:
      return 0;

  }
  check_processor_status(lastValue, reg);
  return 1;
}