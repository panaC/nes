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



void check_processor_status(int32_t lastValue, int8_t value, t_registers *reg)
{

  // TODO : https://www.doc.ic.ac.uk/~eedwards/compsys/arithmetic/index.html#:~:text=Overflow%20Rule%20for%20addition,result%20has%20the%20opposite%20sign.
  // carry should be checked with the sign of the bit 7 and not from a cast to 32bit variable
  if (lastValue > 127 || lastValue < -128)
    reg->p.C = 1; // overflow or underflow
  if (value == 0)
    reg->p.Z = 1; // Zero
  if (value < 0) // or check the bit 7 if equal one then neg
    reg->p.N = 1; // Negatif
  if (lastValue > value) // TODO same as carry
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
    return *memory[arg.value];
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
    VB2(printf("adc opcode absolutey"));
    lastValue = reg->a + addressMode(absolute_y, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 3;
    if ((addr.value & 0x00ff) + reg->x > 0xff)
      cycle += 5;
    else
      cycle += 4;

    break;

   case 0x61: 
    VB2(printf("adc opcode indirectx"));
    lastValue = reg->a + addressMode(indirect_x, addr, reg, memory) + reg->p.C;
    reg->a = (int8_t)lastValue;

    reg->pc += 2;
    cycle += 6;

    break; 
   case 0x71: 
    VB2(printf("adc opcode indirecty"));
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
  check_processor_status(lastValue, reg->a, reg);
  return 1;
}

int and_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0x29:
    VB2(printf("and opcode immediate"));
    reg->a &= *memory[reg->pc + 1];

    cycle += 2;
    reg->pc += 2;
    break;
  case 0x25:
    VB2(printf("and opcode zeropage"));
    reg->a &= addressMode(zero_page, arg, reg, memory);

    cycle += 3;
    reg->pc += 2;
    break;
  
  case 0x35:
    VB2(printf("and opcode zeropagex"));
    reg->a &= addressMode(zero_page_x, arg, reg, memory);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x2d:
    VB2(printf("and opcode absolute"));
    reg->a &= addressMode(absolute, arg, reg, memory);

    cycle += 4;
    reg->pc +=3;
    break;
  
  case 0x3d:
    VB2(printf("and opcode absolutex"));
    reg->a &= addressMode(absolute_x, arg, reg, memory);

    cycle += 4; // Todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x39:
    VB2(printf("and opcode absolutey"));
    reg->a &= addressMode(absolute_y, arg, reg, memory);

    cycle += 4; // todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x21:
    VB2(printf("and opcode indirectx"));
    reg->a &= addressMode(indirect_x, arg, reg, memory);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x31:
    VB2(printf("and opcode indirecty"));
    reg->a &= addressMode(indirect_y, arg, reg, memory);

    cycle += 5; // todo +1 if page crossed
    reg->pc += 2;
    break;
  
  default:
    return 0;
  }

  check_processor_status(reg->a, reg->a, reg);
  return 1;
}

int asl_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0x0a:
    VB2(printf("asl opcode immediate"));
    last_value = reg->a << 1;
    reg->a <<= 1;

    cycle += 2;
    reg->pc += 1;
    check_processor_status(last_value, reg->a, reg);
    break;
  
  case 0x06:
    VB2(printf("asl opcode zeropage"));
    last_value = *memory[arg.lsb] << 1;
    *memory[arg.lsb] <<= 1;

    cycle += 5;
    reg->pc += 2;
    check_processor_status(last_value, *memory[arg.lsb], reg);
    break;

  case 0x16:
    VB2(printf("asl opcode zeropagex"));
    last_value = *memory[arg.lsb + reg->x] << 1;
    *memory[arg.lsb + reg->x] <<= 1;

    cycle += 6;
    reg->pc += 2;
    check_processor_status(last_value, *memory[arg.lsb + reg->x], reg);
    break;

  case 0x0e:
    VB2(printf("asl opcode absolute"));
    last_value = *memory[arg.value] << 1;
    *memory[arg.value] <<= 1;

    cycle += 6;
    reg->pc += 3;
    check_processor_status(last_value, *memory[arg.value], reg);
    break;

  case 0x1e:
    VB2(printf("asl opcode absolutex"));
    last_value = *memory[arg.value + reg->x] << 1;
    *memory[arg.value + reg->x] <<= 1;

    cycle += 7;
    reg->pc += 3;
    check_processor_status(last_value, *memory[arg.value + reg->x], reg);
    break;

  default:
    return 0;
  }

  return 1;
}

int bcc_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0x90) {
    VB2(printf("bcc opcode relative"));

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

int bcs_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0xb0) {
    VB2(printf("bcs opcode relative"));

    if (reg->p.C == 1) {
      reg->pc += *memory[reg->pc + 1];
    } else {
      reg->pc += 2;
    }
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bce_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0xf0) {
    VB2(printf("bce opcode relative"));

    if (reg->p.Z == 1) {
      reg->pc += *memory[reg->pc + 1];
    } else {
      reg->pc += 2;
    }
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bit_opcode(t_registers *reg, t_mem *memory) {

  int8_t value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0x24:
    value = reg->a & *memory[arg.lsb];
    VB2(printf("bit opcode immediate"));

    reg->pc += 2;
    cycle += 3;
    break;

  case 0x2c:
    value = reg->a & addressMode(absolute, arg, reg, memory);
    VB2(printf("bit opcode absolute"));

    reg->pc += 3;
    cycle += 4;
    break;

  default:
    return 0;
  }
  if (value == 0)
    reg->p.Z = 1;
  if (value & 0b100000)
    reg->p.V = 1;
  if (value & 0b1000000)
    reg->p.N = 1;

  return 1;
}

int bmi_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0x30) {
    VB2(printf("bmi opcode relative"));

    if (reg->p.N == 1) {
      reg->pc += *memory[reg->pc + 1];
    } else {
      reg->pc += 2;
    }
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bne_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0xd0) {
    VB2(printf("bne opcode relative"));

    if (reg->p.Z == 0) {
      reg->pc += *memory[reg->pc + 1];
    } else {
      reg->pc += 2;
    }
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bpl_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0x10) {
    VB2(printf("bpl opcode relative"));

    if (reg->p.N == 0) {
      reg->pc += *memory[reg->pc + 1];
    } else {
      reg->pc += 2;
    }
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bvc_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0x50) {
    VB2(printf("bvc opcode relative"));

    if (reg->p.V == 0) {
      reg->pc += *memory[reg->pc + 1];
    } else {
      reg->pc += 2;
    }
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bvs_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0x70) {
    VB2(printf("bvs opcode relative"));

    if (reg->p.V == 1) {
      reg->pc += *memory[reg->pc + 1];
    } else {
      reg->pc += 2;
    }
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int clr_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  switch (op)
  {
  case 0x18:
    // clc
    reg->p.C = 0;
    break;
  
  case 0xd8:
    // cld
    reg->p.D = 0;
    break;

  case 0x58:
    // cli
    reg->p.I = 0;
    break;

  case 0xb8:
    // clv
    reg->p.V = 0;
    break;
  
  default:
    return 0;
  }

  reg->pc += 1;
  cycle += 2;
  return 1;
}

int cmp_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0xc9:
    last_value = reg->a - arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xc5:
    last_value = reg->a - addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xd5:
    last_value = reg->a - addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xcd:
    last_value = reg->a - addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xdd:
    last_value = reg->a - addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  case 0xd9:
    last_value = reg->a - addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;
  
  case 0xc1:
    last_value = reg->a - addressMode(indirect_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 6;
    break;

  case 0xd1:
    last_value = reg->a - addressMode(indirect_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 5; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  if (last_value == 0)
    reg->p.Z = 1;
  if (last_value > 0)
    reg->p.C = 1;
  if (last_value < 0)
    reg->p.N = 1;
  
  return 1;
}

int cpx_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0xe0:
    last_value = reg->x - arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xe4:
    last_value = reg->x - addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xec:
    last_value = reg->x - addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  default:
    return 0;
  }

  if (last_value == 0)
    reg->p.Z = 1;
  if (last_value > 0)
    reg->p.C = 1;
  if (last_value < 0)
    reg->p.N = 1;
  
  return 1;
}

int cpy_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0xc0:
    last_value = reg->y - arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xc4:
    last_value = reg->y - addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xcc:
    last_value = reg->y - addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  default:
    return 0;
  }

  if (last_value == 0)
    reg->p.Z = 1;
  if (last_value > 0)
    reg->p.C = 1;
  if (last_value < 0)
    reg->p.N = 1;
  
  return 1;
}

int dec_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0xc6:
    last_value = *memory[*memory[arg.lsb]] -= 1;

    reg->pc += 2;
    cycle += 5;
    break;
  
  case 0xd6:
    last_value = *memory[*memory[(arg.lsb + reg->x) % 256]] -= 1;

    reg->pc += 2;
    cycle += 6;

  case 0xce:
    last_value = *memory[arg.value] -= 1;

    reg->pc += 3;
    cycle += 6;
  
  case 0xde:
    last_value = *memory[arg.value + reg->x] -= 1;

    reg->pc += 3;
    cycle += 7;
  

  default:
    return 0;
  }

  if (last_value == 0)
    reg->p.Z = 1;
  if (last_value < 0)
    reg->p.N = 1;
  
  return 1;
}

int inc_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0xe6:
    last_value = *memory[*memory[arg.lsb]] += 1;

    reg->pc += 2;
    cycle += 5;
    break;
  
  case 0xf6:
    last_value = *memory[*memory[(arg.lsb + reg->x) % 256]] += 1;

    reg->pc += 2;
    cycle += 6;

  case 0xee:
    last_value = *memory[arg.value] += 1;

    reg->pc += 3;
    cycle += 6;
  
  case 0xfe:
    last_value = *memory[arg.value + reg->x] += 1;

    reg->pc += 3;
    cycle += 7;
  

  default:
    return 0;
  }

  if (last_value == 0)
    reg->p.Z = 1;
  if (last_value < 0)
    reg->p.N = 1;
  
  return 1;
}

int dex_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0xca)
  {
    reg->x -= 1;
    if (reg->x == 0)
      reg->p.Z = 1;
    if (reg->x < 0)
      reg->p.N = 1;
    reg->pc += 1;
    cycle += 2;
    return 1;
  }
  return 0;
}

int dey_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0x88)
  {
    reg->y -= 1;
    if (reg->y == 0)
      reg->p.Z = 1;
    if (reg->y < 0)
      reg->p.N = 1;
    reg->pc += 1;
    cycle += 2;
    return 1;
  }
  return 0;
}

int inx_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0xe8)
  {
    reg->x += 1;
    if (reg->x == 0)
      reg->p.Z = 1;
    if (reg->x < 0)
      reg->p.N = 1;
    reg->pc += 1;
    cycle += 2;
    return 1;
  }
  return 0;
}

int iny_opcode(t_registers *reg, t_mem *memory) {

  uint8_t op = *memory[reg->pc];
  if (op == 0xc8)
  {
    reg->y += 1;
    if (reg->y == 0)
      reg->p.Z = 1;
    if (reg->y < 0)
      reg->p.N = 1;
    reg->pc += 1;
    cycle += 2;
    return 1;
  }
  return 0;
}

int eor_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0x49:
    last_value = reg->a ^ arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0x45:
    last_value = reg->a ^ addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0x55:
    last_value = reg->a ^ addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0x4d:
    last_value = reg->a ^ addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0x5d:
    last_value = reg->a ^ addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  case 0x59:
    last_value = reg->a ^ addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;
  
  case 0x41:
    last_value = reg->a ^ addressMode(indirect_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 6;
    break;

  case 0x51:
    last_value = reg->a ^ addressMode(indirect_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 5; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  if (last_value == 0)
    reg->p.Z = 1;
  if (last_value < 0)
    reg->p.N = 1;
  
  return 1;
}

int jmp_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {
  case 0x4c:
    reg->pc += arg.value; // TODO/QUESTION jump is after opcode jmp or before ?
    cycle += 3;
    break;
  
  case 0x6c:
    reg->pc += 3; // TODO: how to do indirect jump ? https://www.nesdev.org/obelisk-6502-guide/addressing.html#IND
    cycle += 5;
    break;
  
  default:
    return 0;
  }
  
  return 1;
}

int jsr_opcode(t_registers *reg, t_mem *memory) {

  int32_t last_value;
  uint8_t op = *memory[reg->pc];
  union u16 arg = {.lsb = *memory[reg->pc + 1], .msb = *memory[reg->pc + 2]};
  switch (op)
  {

  case 0x6c:
    // TODO : implement jsr with SP
    reg->pc += arg.value;
    cycle += 6;
    break;

  default:
    return 0;
  }

  return 1;
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
    res += and_opcode(reg, memory);
    res += asl_opcode(reg, memory);
    res += bcc_opcode(reg, memory);
    res += bcs_opcode(reg, memory);
    res += bce_opcode(reg, memory);
    res += bit_opcode(reg, memory);
    res += bmi_opcode(reg, memory);
    res += bne_opcode(reg, memory);
    res += bpl_opcode(reg, memory);
    res += bvc_opcode(reg, memory);
    res += bvs_opcode(reg, memory);
    res += clr_opcode(reg, memory);
    res += cmp_opcode(reg, memory);
    res += cpx_opcode(reg, memory);
    res += cpy_opcode(reg, memory);
    res += dec_opcode(reg, memory);
    res += inc_opcode(reg, memory);
    res += dex_opcode(reg, memory);
    res += dey_opcode(reg, memory);
    res += inx_opcode(reg, memory);
    res += iny_opcode(reg, memory);
    res += eor_opcode(reg, memory);
    res += jmp_opcode(reg, memory);
    res += jsr_opcode(reg, memory);
    if (res) {
      // found
    } else {
      VB2(printf("OP=%x not found", op));
      reg->pc++;
    }

    VB4(print_register(reg));
    VB4(printf("cycle=%d", cycle));

  }


}