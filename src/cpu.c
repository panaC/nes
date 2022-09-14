#include <stdio.h>
#include "cpu.h"
#include "debug.h"

// todo:  global variable
// handle State Machine with cycle
uint32_t cycle = 0; 


// TODO : 
// setup a memory checker
// vérfier que l'addresse demandé en mémoire existe bien. Et print une error
// Faire ça avec une macro



void check_processor_status_acc(int32_t lastValue, t_registers *reg)
{

  // TODO : https://www.doc.ic.ac.uk/~eedwards/compsys/arithmetic/index.html#:~:text=Overflow%20Rule%20for%20addition,result%20has%20the%20opposite%20sign.
  // carry should be checked with the sign of the bit 7 and not from a cast to 32bit variable
  if (lastValue > 127 || lastValue < -128)
    reg->p.C = 1; // overflow or underflow
  if (reg->a == 0)
    reg->p.Z = 1; // Zero
  if (reg->a < 0) // or check the bit 7 if equal one then neg
    reg->p.N = 1; // Negatif
  if (lastValue > reg->a) // TODO same as carry
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
  check_processor_status_acc(lastValue, reg);
  return 1;
}

int add_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0x29:
    reg->a &= *memory[reg->pc + 1];

    cycle += 2;
    reg->pc += 2;
    break;
  case 0x25:
    reg->a &= addressMode(zero_page, arg, reg, memory);

    cycle += 3;
    reg->pc += 2;
    break;
  
  case 0x35:
    reg->a &= addressMode(zero_page_x, arg, reg, memory);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x2d:
    reg->a &= addressMode(absolute, arg, reg, memory);

    cycle += 4;
    reg->pc +=3;
    break;
  
  case 0x3d:
    reg->a &= addressMode(absolute_x, arg, reg, memory);

    cycle += 4; // Todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x39:
    reg->a &= addressMode(absolute_y, arg, reg, memory);

    cycle += 4; // todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x21:
    reg->a &= addressMode(indirect_x, arg, reg, memory);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x31:
    reg->a &= addressMode(indirect_y, arg, reg, memory);

    cycle += 5; // todo +1 if page crossed
    reg->pc += 2;
    break;
  
  default:
    return 0;
  }

  check_processor_status_acc(reg->a, reg);
  return 1;
}

int asl_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0x0a:
    last_value = reg->a << 1;
    reg->a <<= 1;

    cycle += 2;
    reg->pc += 1;
    break;
  
  case 0x06:
    *memory[addressMode(zero_page, arg, reg, memory)] <<= 1;
    
    cycle += 5;
    reg->pc += 2;
    break;

  case 0x16:
    *memory[addressMode(zero_page_x, arg, reg, memory)] <<= 1;

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x0e:
    *memory[addressMode(absolute, arg, reg, memory)] <<= 1;

    cycle += 6;
    reg->pc += 3;
    break;

  case 0x1e:
    *memory[addressMode(absolute_x, arg, reg, memory)] <<= 1;

    cycle += 7;
    reg->pc += 3;
    break;

  default:
    return 0;
  }

  check_processor_status_acc(last_value, reg);
  return 1;
}

int bcc_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0x90) {

    if (reg->p.C == 0) {
      reg->pc += *memory[reg->pc + 1];
    } else {
      reg->pc += 2;
    }
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

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
    res += add_opcode(reg, memory);
    res += asl_opcode(reg, memory);
    res += bcc_opcode(reg, memory);
    if (res) {
      // found
    } else {
      VB2(printf("OP=%x not found", op));
      reg->pc++;
    }

    VB4(print_register(reg));

  }


}