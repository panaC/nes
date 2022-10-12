#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cpu.h"
#include "debug.h"
#include "bus.h"
#include "log.h"

#define debug(...) log_x(LOG_CPU, __VA_ARGS__)

// todo:  global variable
// handle State Machine with cycle
uint32_t cycle = 0; 
t_registers __cpu_reg = {
      .pc = 0,
      .sp = 0,
      .p = 0,
      .a = 0,
      .x = 0,
      .y = 0};


// TODO
// implémenter une table de cycle avec les instructions et leur cycles correcspondant 
// permet de créer une machine aà état finis sur l'enchainenement des instructions, la logique de l'instruction sera déclenchée une fois que le cycle est complet (a voir dans le futur si toujours vrais)
// attention car table de cycle pas possible pour les instructions avec page crossed cycle .. comment faire ?


// https://www.pagetable.com/?p=410
void init(t_registers *reg, t_mem *memory) {

  assert(MEM_SIZE == 64 * 1024);
  union u16 addr = {.lsb = readbus(memory, 0xfffc), .msb = readbus(memory, 0xfffd)};
  reg->pc = addr.value;
  reg->a = reg->x = reg->y = 0;
  reg->p.value = 0x34;
  reg->sp = 0xfd;
  *memory[0x4017] = 0;
  *memory[0x4015] = 0;
  bzero(memory[0x4000], 16);
  bzero(memory[0x4010], 4);
}

uint32_t getAddressMode(t_e_mode mode, union u16 arg, t_registers *reg, t_mem *memory) {

  // debug("addressMode arg(LSB=%x,MSB=%x,v=%x)", arg.lsb, arg.msb, arg.value);

  switch (mode)
  {
  case zero_page:
    return arg.lsb;
  case zero_page_x:
    return (arg.lsb + reg->x) % 256;
  case zero_page_y:
    return (arg.lsb + reg->y) % 256;
  case absolute:
    return arg.value;
  case absolute_x:
    return arg.value + reg->x;
  case absolute_y:
    return arg.value + reg->y;
  case indirect_x:
    return readbus(memory, (arg.lsb + reg->x) % 256) + readbus(memory, (arg.lsb + reg->x + 1) % 256) * 256;
  case indirect_y:
    return readbus(memory, arg.lsb) + readbus(memory, (arg.lsb + 1) % 256) * 256 + reg->y;
  default:
    assert(0);
  }
  return 0;
}

uint8_t addressMode(t_e_mode mode, union u16 arg, t_registers *reg, t_mem *memory) {

  return readbus(memory, getAddressMode(mode, arg, reg, memory));
}

void debug_opcode(char* str, t_e_mode mode, uint8_t op, union u16 arg) {

/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *  COMMENT LOG
 * */
  switch (mode)
  {
  case accumulator:
    debug("$%04x    %02x           %s", __cpu_reg.pc, op, str);
    break;
  
  case implied:
    debug("$%04x    %02x           %s", __cpu_reg.pc, op, str);
    break;

  case immediate:
    debug("$%04x    %02x %02x        %s #$%02x", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
    break;
  
  case absolute:
    debug("$%04x    %02x %02x %02x     %s $%04x", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
    break;
  
  case zero_page:
    debug("$%04x    %02x %02x        %s $%02x", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
    break;
  
  case relative:
    debug("$%04x    %02x %02x        %s $%02x", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
    break;

  case absolute_x:
    debug("$%04x    %02x %02x %02x     %s $%04x,X", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
    break;
  
  case absolute_y:
    debug("$%04x    %02x %02x %02x     %s $%04x,Y", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
    break;
  
  case zero_page_x:
    debug("$%04x    %02x %02x        %s $%02x,X", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
    break;

  case zero_page_y:
    debug("$%04x    %02x %02x        %s $%02x,Y", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
    break;

  case indirect:
    debug("$%04x    %02x %02x        %s ($%04x)", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
    break;
  
  case indirect_x:
    debug("$%04x    %02x %02x        %s ($%02x,X)", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);

  case indirect_y:
    debug("$%04x    %02x %02x        %s ($%02x,Y)", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
  
  default:
    break;
  }
}

int adc_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  int16_t result;
  char *str = "ADC";

  switch (op)
  {
  case 0x69: // immediate
    debug_opcode(str, immediate, op, arg);
    result = reg->a + arg.lsb + reg->p.C;

    reg->pc += 2;
    cycle += 2;

    break;
  case 0x65:
    debug_opcode(str, zero_page, op, arg);
    result = reg->a + addressMode(zero_page, arg, reg, memory) + reg->p.C;

    reg->pc += 2;
    cycle += 3;

    break;
  case 0x75: 
    debug_opcode(str, zero_page_x, op, arg);
    result = reg->a + addressMode(zero_page_x, arg, reg, memory) + reg->p.C;

    reg->pc += 2;
    cycle += 4;

    break;
  case 0x6d: 
    debug_opcode(str, absolute, op, arg);
    result = reg->a + addressMode(absolute, arg, reg, memory) + reg->p.C;

    reg->pc += 3;
    cycle += 4;

    break;
  case 0x7d: 
    debug_opcode(str, absolute_x, op, arg);
    result = reg->a + addressMode(absolute_x, arg, reg, memory) + reg->p.C;

    reg->pc += 3;
    if ((arg.value & 0x00ff) + reg->x > 0xff)
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
    debug_opcode(str, absolute_y, op, arg);
    result = reg->a + addressMode(absolute_y, arg, reg, memory) + reg->p.C;

    reg->pc += 3;
    if ((arg.value & 0x00ff) + reg->x > 0xff)
      cycle += 5;
    else
      cycle += 4;

    break;

   case 0x61: 
    debug_opcode(str, indirect_x, op, arg);
    result = reg->a + addressMode(indirect_x, arg, reg, memory) + reg->p.C;

    reg->pc += 2;
    cycle += 6;

    break; 
   case 0x71: 
    debug_opcode(str, indirect_y, op, arg);
    result = reg->a + addressMode(indirect_y, arg, reg, memory) + reg->p.C;

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

  reg->a = result;

  reg->p.Z = !result;
  reg->p.N = (result & 0x80) >> 7; 

  // http://www.6502.org/tutorials/vflag.html
  reg->p.C = (result & 0x100) >> 8;
  // As stated above, the second purpose of the carry flag
  // is to indicate when the result of the
  // addition or subtraction is outside the range 0 to 255, specifically:
    // When the addition result is 0 to 255, the carry is cleared.
    // When the addition result is greater than 255, the carry is set.
  
  // http://www.6502.org/tutorials/vflag.html
  reg->p.V = result >= 0 ? (result & 0x100) >> 8 : !((result & 0x80) >> 7);
  
  return 1;
}

int and_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "AND";

  switch (op)
  {
  case 0x29:
    debug_opcode(str, immediate, op, arg);;
    reg->a &= readbus(memory, reg->pc + 1); // or arg.lsb

    cycle += 2;
    reg->pc += 2;
    break;
  case 0x25:
    debug_opcode(str, zero_page, op, arg);;
    reg->a &= addressMode(zero_page, arg, reg, memory);

    cycle += 3;
    reg->pc += 2;
    break;
  
  case 0x35:
    debug_opcode(str, zero_page_x, op, arg);;
    reg->a &= addressMode(zero_page_x, arg, reg, memory);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x2d:
    debug_opcode(str, absolute, op, arg);;
    reg->a &= addressMode(absolute, arg, reg, memory);

    cycle += 4;
    reg->pc +=3;
    break;
  
  case 0x3d:
    debug_opcode(str, absolute_x, op, arg);;
    reg->a &= addressMode(absolute_x, arg, reg, memory);

    cycle += 4; // Todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x39:
    debug_opcode(str, absolute_y, op, arg);;
    reg->a &= addressMode(absolute_y, arg, reg, memory);

    cycle += 4; // todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x21:
    debug_opcode(str, indirect_x, op, arg);;
    reg->a &= addressMode(indirect_x, arg, reg, memory);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x31:
    debug_opcode(str, indirect_y, op, arg);;
    reg->a &= addressMode(indirect_y, arg, reg, memory);

    cycle += 5; // todo +1 if page crossed
    reg->pc += 2;
    break;
  
  default:
    return 0;
  }
  reg->p.Z = !!reg->a;
  reg->p.N = reg->a >> 7; 

  return 1;
}

int asl_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "ASL";

  uint16_t result;
  switch (op)
  {
  case 0x0a:
    debug_opcode(str, accumulator, op, arg);;
    result = reg->a << 1;
    reg->a <<= 1;

    cycle += 2;
    reg->pc += 1;
    break;
  
  case 0x06:
    debug_opcode(str, zero_page, op, arg);;
    result = readbus(memory, getAddressMode(zero_page, arg, reg, memory)) << 1;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), result);

    cycle += 5;
    reg->pc += 2;
    break;

  case 0x16:
    debug_opcode(str, zero_page_x, op, arg);;
    result = readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) << 1;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), result);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x0e:
    debug_opcode(str, absolute, op, arg);;
    result = readbus(memory, getAddressMode(absolute, arg, reg, memory)) << 1;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), result);

    cycle += 6;
    reg->pc += 3;
    break;

  case 0x1e:
    debug_opcode(str, absolute_x, op, arg);;
    result = readbus(memory, getAddressMode(absolute_x, arg, reg, memory)) << 1;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), result);

    cycle += 7;
    reg->pc += 3;
    break;

  default:
    return 0;
  }

  reg->p.Z = !result;
  reg->p.N = (result & 0x80) >> 7;
  reg->p.C = (result & 0x100) >> 8; // Set to contents of old bit 7

  return 1;
}

int bcc_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0x90) {
    debug_opcode("BCC", relative, op, (union u16){.value = 0});;

    if (reg->p.C == 0) {
      reg->pc += (int8_t)readbus(memory, reg->pc + 1);
    }
    reg->pc += 2;
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bcs_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0xb0) {
    debug_opcode("BCS", relative, op, (union u16){.value = 0});;

    if (reg->p.C == 1) {
      reg->pc += (int8_t)readbus(memory, reg->pc + 1);
    }
    reg->pc += 2;
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int beq_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0xf0) {
    debug_opcode("BEQ", relative, op, (union u16){.value = 0});;

    if (reg->p.Z == 1) {
      reg->pc += (int8_t)readbus(memory, reg->pc + 1);
    }
    reg->pc += 2; // in both case
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bit_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "BIT";

  int8_t value;
  switch (op)
  {
  case 0x24:
    debug_opcode(str, zero_page, op, arg);;
    value = reg->a & readbus(memory, arg.lsb);

    reg->pc += 2;
    cycle += 3;
    break;

  case 0x2c:
    debug_opcode(str, absolute, op, arg);;
    value = reg->a & addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  default:
    return 0;
  }
  reg->p.Z = !value;
  reg->p.V = (value & 0x40) >> 6;
  reg->p.N = (value & 0x80) >> 7;

  return 1;
}

int bmi_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0x30) {
    debug_opcode("BMI", relative, op, (union u16){.value = 0});;

    if (reg->p.N == 1) {
      reg->pc += (int8_t)readbus(memory, reg->pc + 1);
    }
    reg->pc += 2;
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bne_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0xd0) {
    debug_opcode("BNE", relative, op, (union u16){.value = 0});;

    if (reg->p.Z == 0) {
      reg->pc += (int8_t)readbus(memory, reg->pc + 1);
    }
    reg->pc += 2;
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bpl_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0x10) {
    debug_opcode("BPL", relative, op, (union u16){.value = 0});;

    if (reg->p.N == 0) {
      reg->pc += (int8_t)readbus(memory, reg->pc + 1);
    }
    reg->pc += 2;
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bvc_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0x50) {
    debug_opcode("BVC", relative, op, (union u16){.value = 0});;

    if (reg->p.V == 0) {
      reg->pc += (int8_t)readbus(memory, reg->pc + 1);
    }
    reg->pc += 2;
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int bvs_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0x70) {
    debug_opcode("BVS", relative, op, (union u16){.value = 0});;

    if (reg->p.V == 1) {
      reg->pc += (int8_t)readbus(memory, reg->pc + 1);
    }
    reg->pc += 2;
    cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
    return 1;
  }
  return 0;
}

int clr_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  switch (op)
  {
  case 0x18:
    debug_opcode("CLC", implied, op, (union u16){.value = 0});;
    // clc
    reg->p.C = 0;
    break;
  
  case 0xd8:
    debug_opcode("CLD", implied, op, (union u16){.value = 0});;
    // cld
    reg->p.D = 0;
    break;

  case 0x58:
    debug_opcode("CLI", implied, op, (union u16){.value = 0});;
    // cli
    reg->p.I = 0;
    break;

  case 0xb8:
    debug_opcode("CLV", implied, op, (union u16){.value = 0});;
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

int sbc_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "SBC";

  int16_t result;
  switch (op)
  {
  case 0xe9:
    debug_opcode(str, immediate, op, arg);;
    result = reg->a - arg.lsb - (1 - reg->p.C);
    reg->pc += 2;
    cycle += 2;
    break;

  case 0xe5:
    debug_opcode(str, zero_page, op, arg);;
    result = reg->a - addressMode(zero_page, arg, reg, memory) - (1 - reg->p.C);
    reg->pc += 2;
    cycle += 3;
    break;

  case 0xf5:
    debug_opcode(str, zero_page_x, op, arg);;
    result = reg->a - addressMode(zero_page_x, arg, reg, memory) - (1 - reg->p.C);
    reg->pc += 2;
    cycle += 4;
    break;
 
  case 0xed:
    debug_opcode(str, absolute, op, arg);;
    result = reg->a - addressMode(absolute, arg, reg, memory) - (1 - reg->p.C);
    reg->pc += 3;
    cycle += 4;
    break; 

  case 0xfd:
    debug_opcode(str, absolute_x, op, arg);;
    result = reg->a - addressMode(absolute_x, arg, reg, memory) - (1 - reg->p.C);
    reg->pc += 3;
    cycle += 4; // TODO: +1 if page crossed
    break;

  case 0xf9:
    debug_opcode(str, absolute_y, op, arg);;
    result = reg->a - addressMode(absolute_y, arg, reg, memory) - (1 - reg->p.C);
    reg->pc += 3;
    cycle += 4; // TODO: +1 if page crossed
    break;

  case 0xe1:
    debug_opcode(str, indirect_x, op, arg);;
    result = reg->a - addressMode(indirect_x, arg, reg, memory) - (1 - reg->p.C);
    reg->pc += 2;
    cycle += 6;
    break;

  case 0xf1:
    debug_opcode(str, indirect_y, op, arg);;
    result = reg->a - addressMode(indirect_y, arg, reg, memory) - (1 - reg->p.C);
    reg->pc += 2;
    cycle += 6; // TODO: +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->a = result;

  reg->p.Z = !result;
  reg->p.N = (result & 0x80) >> 7; 

  // http://www.6502.org/tutorials/vflag.html
  reg->p.C = !((result & 0x80) >> 7);
  // As stated above, the second purpose of the carry flag
  // is to indicate when the result of the
  // addition or subtraction is outside the range 0 to 255, specifically:
    // When the subtraction result is 0 to 255, the carry is set.
    // When the subtraction result is less than 0, the carry is cleared.

  // http://www.6502.org/tutorials/vflag.html
  reg->p.V = result >= 0 ? (result & 0x100) >> 8 : !((result & 0x80) >> 7);

  return 1;
}

int set_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  switch (op)
  {
  case 0x38:
    debug_opcode("SEC", implied, op, (union u16){.value = 0});;
    // sec
    reg->p.C = 1;
    break;
  
  case 0xf8:
    // sed
    debug_opcode("SED", implied, op, (union u16){.value = 0});;
    reg->p.D = 1;
    break;

  case 0x78:
    // sei
    debug_opcode("SEI", implied, op, (union u16){.value = 0});;
    reg->p.I = 1;
    break;

  default:
    return 0;
  }

  reg->pc += 1;
  cycle += 2;
  return 1;
}

int trs_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  int8_t result;
  switch (op)
  {
  case 0xaa:
    debug_opcode("TAX", implied, op, (union u16){.value = 0});;
    // tax
    reg->x = reg->a;
    reg->p.Z = !reg->x;
    reg->p.N = (reg->x & 0x80) >> 7; 
    break;
  
  case 0xa8:
    debug_opcode("TAY", implied, op, (union u16){.value = 0});;
    // tay
    reg->y = reg->a;
    reg->p.Z = !reg->y;
    reg->p.N = (reg->y & 0x80) >> 7; 
    break;

  case 0xba:
    debug_opcode("TSX", implied, op, (union u16){.value = 0});;
    // tsx
    reg->x = reg->sp;
    reg->p.Z = !reg->x;
    reg->p.N = (reg->x & 0x80) >> 7; 
    break;

  case 0x8a:
    debug_opcode("TXA", implied, op, (union u16){.value = 0});;
    // txa
    reg->a = reg->x;
    reg->p.Z = !reg->a;
    reg->p.N = (reg->a & 0x80) >> 7;
    break;
  
  case 0x9a:
    debug_opcode("TXS", implied, op, (union u16){.value = 0});;
    // txs
    reg->sp = reg->x;
    // no check processor status
    break;
  
  case 0x98:
    debug_opcode("TYA", implied, op, (union u16){.value = 0});;
    // tya
    reg->a = reg->y;
    reg->p.Z = !reg->a;
    reg->p.N = (reg->a & 0x80) >> 7; 
    break;

  default:
    return 0;
  }

  reg->pc += 1;
  cycle += 2;
  return 1;
}

int cmp_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "CMP";

  int8_t mem;
  switch (op)
  {
  case 0xc9:
    debug_opcode(str, immediate, op, arg);;
    mem = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xc5:
    debug_opcode(str, zero_page, op, arg);;
    mem = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xd5:
    debug_opcode(str, zero_page_x, op, arg);;
    mem = addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xcd:
    debug_opcode(str, absolute, op, arg);;
    mem = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xdd:
    debug_opcode(str, absolute_x, op, arg);;
    mem = addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  case 0xd9:
    debug_opcode(str, absolute_y, op, arg);;
    mem = addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;
  
  case 0xc1:
    debug_opcode(str, indirect_x, op, arg);;
    mem = addressMode(indirect_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 6;
    break;

  case 0xd1:
    debug_opcode(str, indirect_y, op, arg);;
    mem = addressMode(indirect_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 5; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = reg->a == mem ? 1 : 0;
  reg->p.N = reg->a < mem ? 1 : 0;
  reg->p.C = reg->a >= mem ? 1 : 0;

  return 1;
}

int cpx_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "CPX";

  int32_t last_value;
  int8_t mem;
  switch (op)
  {
  case 0xe0:
    debug_opcode(str, immediate, op, arg);;
    mem = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xe4:
    debug_opcode(str, zero_page, op, arg);;
    mem = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xec:
    debug_opcode(str, absolute, op, arg);;
    mem = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  default:
    return 0;
  }

  reg->p.Z = reg->x == mem ? 1 : 0;
  reg->p.N = reg->x < mem ? 1 : 0;
  reg->p.C = reg->x >= mem ? 1 : 0;

  return 1;
}

int cpy_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "CPY";

  int8_t mem;
  switch (op)
  {
  case 0xc0:
    debug_opcode(str, immediate, op, arg);;
    mem = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xc4:
    debug_opcode(str, zero_page, op, arg);;
    mem = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xcc:
    debug_opcode(str, absolute, op, arg);;
    mem = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  default:
    return 0;
  }

  reg->p.Z = reg->y == mem ? 1 : 0;
  reg->p.N = reg->y < mem ? 1 : 0;
  reg->p.C = reg->y >= mem ? 1 : 0;


  
  return 1;
}

int dec_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "DEC";

  int8_t result;
  switch (op)
  {
  case 0xc6:
    debug_opcode(str, zero_page, op, arg);;
    result = (int8_t)readbus(memory, getAddressMode(zero_page, arg, reg, memory)) - 1;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), result);

    reg->pc += 2;
    cycle += 5;
    break;
  
  case 0xd6:
    debug_opcode(str, zero_page_x, op, arg);;
    result = (int8_t)readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) - 1;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), result);

    reg->pc += 2;
    cycle += 6;

  case 0xce:
    debug_opcode(str, absolute, op, arg);;
    result = (int8_t)readbus(memory, getAddressMode(absolute, arg, reg, memory)) - 1;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), result);

    reg->pc += 3;
    cycle += 6;
  
  case 0xde:
    debug_opcode(str, absolute_x, op, arg);;
    result = (int8_t)readbus(memory, getAddressMode(absolute_x, arg, reg, memory)) - 1;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), result);

    reg->pc += 3;
    cycle += 7;
  

  default:
    return 0;
  }

  reg->p.Z = result == 0 ? 1 : 0;
  reg->p.N = result < 0 ? 1 : 0;

  return 1;
}

int inc_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "INC";

  int8_t result;
  switch (op)
  {
  case 0xe6:
    debug_opcode(str, zero_page, op, arg);
    result = (int8_t)readbus(memory, getAddressMode(zero_page, arg, reg, memory)) + 1;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), result);

    reg->pc += 2;
    cycle += 5;
    break;
  
  case 0xf6:
    debug_opcode(str, zero_page_x, op, arg);;
    result = (int8_t)readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) + 1;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), result);

    reg->pc += 2;
    cycle += 6;

  case 0xee:
    debug_opcode(str, absolute, op, arg);;
    result = (int8_t)readbus(memory, getAddressMode(absolute, arg, reg, memory)) + 1;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), result);

    reg->pc += 3;
    cycle += 6;
  
  case 0xfe:
    debug_opcode(str, absolute_x, op, arg);;
    result = (int8_t)readbus(memory, getAddressMode(absolute_x, arg, reg, memory)) + 1;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), result);

    reg->pc += 3;
    cycle += 7;
  

  default:
    return 0;
  }

  reg->p.Z = result == 0 ? 1 : 0;
  reg->p.N = result < 0 ? 1 : 0;
  
  return 1;
}

int dex_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0xca)
  {
    debug_opcode("DEX", implied, op, (union u16){.value = 0});;
    reg->x -= 1;
    reg->p.Z = reg->x == 0 ? 1 : 0;
    reg->p.N = reg->x < 0 ? 1 : 0;
    reg->pc += 1;
    cycle += 2;
    return 1;
  }
  return 0;
}

int dey_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0x88)
  {
    debug_opcode("DEY", implied, op, (union u16){.value = 0});;
    reg->y -= 1;
    reg->p.Z = reg->y == 0 ? 1 : 0;
    reg->p.N = reg->y < 0 ? 1 : 0;
    reg->pc += 1;
    cycle += 2;
    return 1;
  }
  return 0;
}

int inx_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0xe8)
  {
    debug_opcode("INX", implied, op, (union u16){.value = 0});;
    reg->x += 1;
    reg->p.Z = reg->x == 0 ? 1 : 0;
    reg->p.N = reg->x < 0 ? 1 : 0;
    reg->pc += 1;
    cycle += 2;
    return 1;
  }
  return 0;
}

int iny_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0xc8)
  {
    debug_opcode("INY", implied, op, (union u16){.value = 0});;
    reg->y += 1;
    reg->p.Z = reg->y == 0 ? 1 : 0;
    reg->p.N = reg->y < 0 ? 1 : 0;
    reg->pc += 1;
    cycle += 2;
    return 1;
  }
  return 0;
}

int eor_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "EOR";

  int8_t result;
  switch (op)
  {
  case 0x49:
    debug_opcode(str, immediate, op, arg);;
    result = reg->a ^ arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0x45:
    debug_opcode(str, zero_page, op, arg);;
    result = reg->a ^ (int8_t)addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0x55:
    debug_opcode(str, zero_page_x, op, arg);;
    result = reg->a ^ (int8_t)addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0x4d:
    debug_opcode(str, absolute, op, arg);;
    result = reg->a ^ (int8_t)addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0x5d:
    debug_opcode(str, absolute_x, op, arg);;
    result = reg->a ^ (int8_t)addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  case 0x59:
    debug_opcode(str, absolute_y, op, arg);;
    result = reg->a ^ (int8_t)addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;
  
  case 0x41:
    debug_opcode(str, indirect_x, op, arg);;
    result = reg->a ^ (int8_t)addressMode(indirect_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 6;
    break;

  case 0x51:
    debug_opcode(str, indirect_y, op, arg);;
    result = reg->a ^ (int8_t)addressMode(indirect_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 5; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = result == 0 ? 1 : 0;
  reg->p.N = result < 0 ? 1 : 0;

  return 1;
}

int jmp_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "JMP";

  switch (op)
  {
  case 0x4c:
    debug_opcode(str, absolute, op, arg);;
    reg->pc = arg.value;
    cycle += 3;
    break;
  
  case 0x6c:
    debug_opcode(str, indirect, op, arg);
    assert(0);
    reg->pc += 3; // TODO: how to do indirect jump ? https://www.nesdev.org/obelisk-6502-guide/addressing.html#IND
    cycle += 5;
    break;
  
  default:
    return 0;
  }
  
  return 1;
}

int lda_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "LDA";

  switch (op)
  {
  case 0xa9:
    debug_opcode(str, immediate, op, arg);;
    reg->a = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xa5:
    debug_opcode(str, zero_page, op, arg);;
    reg->a = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xb5:
    debug_opcode(str, zero_page_x, op, arg);;
    reg->a = addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xad:
    debug_opcode(str, absolute, op, arg);;
    reg->a = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xbd:
    debug_opcode(str, absolute_x, op, arg);;
    reg->a = addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  case 0xb9:
    debug_opcode(str, absolute_y, op, arg);;
    reg->a = addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;
  
  case 0xa1:
    debug_opcode(str, indirect_x, op, arg);;
    reg->a = addressMode(indirect_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 6;
    break;

  case 0xb1:
    debug_opcode(str, indirect_y, op, arg);;
    reg->a = addressMode(indirect_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 5; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = reg->a == 0 ? 1 : 0;
  reg->p.N = reg->a < 0 ? 1 : 0;

  return 1;
}

int ldx_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "LDX";

  switch (op)
  {
  case 0xa2:
    debug_opcode(str, immediate, op, arg);;
    reg->x = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xa6:
    debug_opcode(str, zero_page, op, arg);;
    reg->x = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xb6:
    debug_opcode(str, zero_page_y, op, arg);;
    reg->x = addressMode(zero_page_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xae:
    debug_opcode(str, absolute, op, arg);;
    reg->x = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xbe:
    debug_opcode(str, absolute_y, op, arg);;
    reg->x = addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = reg->x == 0 ? 1 : 0;
  reg->p.N = reg->x < 0 ? 1 : 0;

  return 1;
}

int ldy_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "LDY";

  switch (op)
  {
  case 0xa0:
    debug_opcode(str, immediate, op, arg);;
    reg->y = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xa4:
    debug_opcode(str, zero_page, op, arg);;
    reg->y = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xb4:
    debug_opcode(str, zero_page_x, op, arg);;
    reg->y = addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xac:
    debug_opcode(str, absolute, op, arg);;
    reg->y = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xbc:
    debug_opcode(str, absolute_x, op, arg);;
    reg->y = addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = reg->y == 0 ? 1 : 0;
  reg->p.N = reg->y < 0 ? 1 : 0;

  return 1;
}

int lsr_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "LSR";

  uint16_t result;
  uint8_t mem;
  switch (op)
  {
  case 0x4a:
    debug_opcode(str, immediate, op, arg);;
    mem = reg->a;
    result = mem >> 1;
    reg->a >>= 1;

    cycle += 2;
    reg->pc += 1;
    break;
  
  case 0x46:
    debug_opcode(str, zero_page, op, arg);;
    mem = addressMode(zero_page, arg, reg, memory);
    result = mem >> 1;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), result);

    cycle += 5;
    reg->pc += 2;
    // TODO
    break;

  case 0x56:
    debug_opcode(str, zero_page_x, op, arg);;
    mem = addressMode(zero_page_x, arg, reg, memory);
    result = mem >> 1;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), result);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x4e:
    debug_opcode(str, absolute, op, arg);;
    mem = addressMode(absolute, arg, reg, memory);
    result = mem >> 1;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), result);

    cycle += 6;
    reg->pc += 3;
    break;

  case 0x5e:
    debug_opcode(str, absolute_x, op, arg);;
    mem = addressMode(absolute_x, arg, reg, memory);
    result = mem >> 1;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), result);

    cycle += 7;
    reg->pc += 3;
    break;

  default:
    return 0;
  }

  reg->p.Z = !result;
  reg->p.N = (result & 0x80) >> 7;
  reg->p.C = mem & 0x1; // 	Set to contents of old bit 0

  return 1;
}

int ora_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "ORA";

  switch (op)
  {
  case 0x09:
    debug_opcode(str, immediate, op, arg);;
    reg->a |= readbus(memory, reg->pc + 1);

    cycle += 2;
    reg->pc += 2;
    break;
  case 0x05:
    debug_opcode(str, zero_page, op, arg);;
    reg->a |= addressMode(zero_page, arg, reg, memory);

    cycle += 3;
    reg->pc += 2;
    break;
  
  case 0x15:
    debug_opcode(str, zero_page_x, op, arg);;
    reg->a |= addressMode(zero_page_x, arg, reg, memory);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x0d:
    debug_opcode(str, absolute, op, arg);;
    reg->a &= addressMode(absolute, arg, reg, memory);

    cycle += 4;
    reg->pc += 3;
    break;
  
  case 0x1d:
    debug_opcode(str, absolute_x, op, arg);;
    reg->a &= addressMode(absolute_x, arg, reg, memory);

    cycle += 4; // Todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x19:
    debug_opcode(str, absolute_y, op, arg);;
    reg->a |= addressMode(absolute_y, arg, reg, memory);

    cycle += 4; // todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x01:
    debug_opcode(str, indirect_x, op, arg);;
    reg->a |= addressMode(indirect_x, arg, reg, memory);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x11:
    debug_opcode(str, indirect_y, op, arg);;
    reg->a |= addressMode(indirect_y, arg, reg, memory);

    cycle += 5; // todo +1 if page crossed
    reg->pc += 2;
    break;
  
  default:
    return 0;
  }

  reg->p.Z = !reg->a;
  reg->p.N = (reg->a & 0x80) >> 7;
  return 1;
}

int sta_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "STA";

  switch (op)
  {
  case 0x85:
    debug_opcode(str, zero_page, op, arg);;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), reg->a);

    cycle += 3;
    reg->pc += 2;
    break;
  case 0x95:
    debug_opcode(str, zero_page_x, op, arg);;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), reg->a);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x8d:
    debug_opcode(str, absolute, op, arg);;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), reg->a);

    cycle += 4;
    reg->pc += 3;
    break;
  
  case 0x9d:
    debug_opcode(str, absolute_x, op, arg);;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), reg->a);

    cycle += 5;
    reg->pc += 3;
    break;

  case 0x99:
    debug_opcode(str, absolute_y, op, arg);;
    writebus(memory, getAddressMode(absolute_y, arg, reg, memory), reg->a);

    cycle += 5;
    reg->pc += 3;
    break;

  case 0x81:
    debug_opcode(str, indirect_x, op, arg);;
    writebus(memory, getAddressMode(indirect_x, arg, reg, memory), reg->a);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x91:
    debug_opcode(str, indirect_y, op, arg);;
    writebus(memory, getAddressMode(indirect_y, arg, reg, memory), reg->a);

    cycle += 6;
    reg->pc += 2;
    break;
  
  default:
    return 0;
  }

  return 1;
}

int stx_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "STX";

  switch (op)
  {
  case 0x86:
    debug_opcode(str, zero_page, op, arg);;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), reg->x);

    cycle += 3;
    reg->pc += 2;
    break;
  case 0x96:
    debug_opcode(str, zero_page_y, op, arg);;
    writebus(memory, getAddressMode(zero_page_y, arg, reg, memory), reg->x);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x8e:
    debug_opcode(str, absolute, op, arg);;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), reg->x);

    cycle += 4;
    reg->pc += 3;
    break;
  
  default:
    return 0;
  }

  return 1;
}

int sty_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "STY";

  switch (op)
  {
  case 0x84:
    debug_opcode(str, immediate, op, arg);;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), reg->y);

    cycle += 3;
    reg->pc += 2;
    break;

  case 0x94:
    debug_opcode(str, zero_page_x, op, arg);;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), reg->y);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x8c:
    debug_opcode(str, absolute, op, arg);;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), reg->y);

    cycle += 4;
    reg->pc += 3;
    break;
  
  default:
    return 0;
  }

  return 1;
}

int rol_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "ROL";

  int last_value, oldbit = 0;
  switch (op)
  {
  case 0x2a:
    debug_opcode(str, immediate, op, arg);;
    oldbit = (reg->a & 0x80) >> 7;
    reg->a <<= 1;
    reg->a = reg->a | reg->p.C;
    reg->p.C = oldbit;

    reg->pc += 1;
    cycle += 2;
    break;

  case 0x26:
    debug_opcode(str, zero_page, op, arg);;
    oldbit = (addressMode(zero_page, arg, reg, memory) & 0x80) >> 7;
    reg->a <<= 1;
    last_value = readbus(memory, getAddressMode(zero_page, arg, reg, memory)) | reg->p.C;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 2;
    cycle += 5;
    break;

  case 0x36:
    debug_opcode(str, zero_page_x, op, arg);;
    oldbit = (addressMode(zero_page_x, arg, reg, memory) & 0x80) >> 7;
    reg->a <<= 1;
    last_value = readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) | reg->p.C;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 2;
    cycle += 6;
    break;

  case 0x2e:
    debug_opcode(str, absolute, op, arg);;
    oldbit = (addressMode(absolute, arg, reg, memory) & 0x80) >> 7;
    reg->a <<= 1;
    last_value = readbus(memory, getAddressMode(absolute, arg, reg, memory)) | reg->p.C;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 3;
    cycle += 6;
    break;
 
  case 0x3e:
    debug_opcode(str, absolute_x, op, arg);;
    oldbit = (addressMode(absolute_x, arg, reg, memory) & 0x80) >> 7;
    reg->a <<= 1;
    last_value = readbus(memory, getAddressMode(absolute_x, arg, reg, memory)) | reg->p.C;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 3;
    cycle += 7;
    break; 

  default:
    return 0;
  }

  reg->p.Z = last_value == 0 ? 1 : 0;
  reg->p.N = last_value < 0 ? 1 : 0;

  return 1;
}

int ror_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {
  char *str = "ROR";

  int last_value, oldbit = 0;
  switch (op)
  {
  case 0x6a:
    debug_opcode(str, immediate, op, arg);;
    oldbit = reg->a & 0x01;
    reg->a >>= 1;
    last_value = reg->a |= reg->p.C << 7;
    reg->p.C = oldbit;

    reg->pc += 1;
    cycle += 2;
    break;

  case 0x66:
    debug_opcode(str, zero_page, op, arg);;
    oldbit = addressMode(zero_page, arg, reg, memory) & 0x01;
    reg->a >>= 1;
    last_value = readbus(memory, getAddressMode(zero_page, arg, reg, memory)) | (reg->p.C << 7);
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 2;
    cycle += 5;
    break;

  case 0x76:
    debug_opcode(str, zero_page_x, op, arg);;
    oldbit = addressMode(zero_page_x, arg, reg, memory) & 0x01;
    reg->a >>= 1;
    last_value = readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) | (reg->p.C << 7);
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 2;
    cycle += 6;
    break;

  case 0x6e:
    debug_opcode(str, absolute, op, arg);;
    oldbit = addressMode(absolute, arg, reg, memory) & 0x01;
    reg->a >>= 1;
    last_value = readbus(memory, getAddressMode(absolute, arg, reg, memory)) | (reg->p.C << 7);
    writebus(memory, getAddressMode(absolute, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 3;
    cycle += 6;
    break;
 
  case 0x7e:
    debug_opcode(str, absolute_x, op, arg);;
    oldbit = addressMode(absolute_x, arg, reg, memory) & 0x01;
    reg->a >>= 1;
    last_value = readbus(memory, getAddressMode(absolute_x, arg, reg, memory)) | (reg->p.C << 7);
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 3;
    cycle += 7;
    break; 

  default:
    return 0;
  }

  reg->p.Z = last_value == 0 ? 1 : 0;
  reg->p.N = last_value < 0 ? 1 : 0;

  return 1;;
}

int jsr_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  union u16 pc = {0};
  if (op == 0x20) {
    debug_opcode("JSR", absolute, op, arg);;
    pc.value = reg->pc += 3;
    writebus(memory, 0x01ff - reg->sp, pc.msb);
    reg->sp--;
    writebus(memory, 0x01ff - reg->sp, pc.lsb);
    reg->sp--;
    reg->pc = arg.value;
    cycle += 6;
    return 1;
  }

  return 0;
}

int rts_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  union u16 pc = {0};
  if (op == 0x60) {
    debug_opcode("RTS", implied, op, arg);;
    reg->sp++;
    pc.lsb = readbus(memory, 0x01ff - reg->sp);
    reg->sp++;
    pc.msb = readbus(memory, 0x01ff - reg->sp);
    reg->pc = pc.value;
    cycle += 6;
    return 1;
  }

  return 0;
}

int psp_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  switch (op)
  {
  case 0x48:
    debug_opcode("PHA", implied, op, (union u16){.value = 0});;
    writebus(memory, 0x01ff - reg->sp, reg->a);
    reg->sp--;
    reg->pc += 1;
    cycle += 3;
    break;
  
  case 0x08:
    debug_opcode("PHP", implied, op, (union u16){.value = 0});;
    writebus(memory, 0x01ff - reg->sp, reg->p.value);
    reg->sp--;
    reg->pc += 1;
    cycle += 3;
    break;

  case 0x68:
    debug_opcode("PLA", implied, op, (union u16){.value = 0});;
    reg->sp++;
    reg->a = readbus(memory, 0x01ff - reg->sp);
    reg->p.Z = !reg->a;
    reg->p.N = (reg->a & 0x80) >> 7;
    reg->pc += 1;
    cycle += 3;
    break;

  case 0x28:
    debug_opcode("PLP", implied, op, (union u16){.value = 0});;
    reg->sp++;
    reg->p.value = readbus(memory, 0x01ff - reg->sp);
    reg->pc += 1;
    cycle += 4;
    break;
  
  default:
    return 0;
  }

  return 1;
}

int brk_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0) {
    debug("BRK");
    irq(reg, memory);
    reg->p.B = 1;

    return 1;
  }
  return 0;
}

int nop_opcode(t_registers *reg, t_mem *memory, uint8_t op) {
  if (op == 0xea) {
    debug("NOP");
    cycle += 2;
    reg->pc += 1;
    return 1;
  }
  return 0;
}

// global function
void reset(t_registers *reg, t_mem *memory) {
  // https://www.pagetable.com/?p=410
  // TODO
}

void irq(t_registers *reg, t_mem *memory) {
  union u16 pc = {.value = reg->pc};
  pc.value = reg->pc += 2;
  *memory[0x01ff - reg->sp] = pc.msb;
  reg->sp--;
  *memory[0x01ff - reg->sp] = pc.lsb;
  reg->sp--;
  *memory[0x01ff - reg->sp] = reg->p.value;
  reg->sp--;
  union u16 brk = {.lsb = *memory[0xfffe], .msb = *memory[0xffff]};
  reg->pc = brk.value;
  reg->p.B = 0; // clear b flags

  cycle += 7;
}

int cpu_exec(t_mem *memory, t_registers *reg) {

  int res = 0;
  uint8_t op = readbus(memory, reg->pc);
  union u16 addr = readbus16(memory, reg->pc + 1);
#ifdef DEBUG_CPU
  if (op == 0)
  {
    // break;
    debug("DEBUG_CPU BREAK");
    return -1;
  }
#else
  res += brk_opcode(reg, memory, op);
#endif
  res += adc_opcode(reg, memory, op, addr);
  res += and_opcode(reg, memory, op, addr);
  res += asl_opcode(reg, memory, op, addr);
  res += bcc_opcode(reg, memory, op);
  res += bcs_opcode(reg, memory, op);
  res += beq_opcode(reg, memory, op);
  res += bit_opcode(reg, memory, op, addr);
  res += bmi_opcode(reg, memory, op);
  res += bne_opcode(reg, memory, op);
  res += bpl_opcode(reg, memory, op);
  res += bvc_opcode(reg, memory, op);
  res += bvs_opcode(reg, memory, op);
  res += clr_opcode(reg, memory, op); // clear opcodes
  res += set_opcode(reg, memory, op); // set opcodes
  res += trs_opcode(reg, memory, op); // transfer opcodes
  res += cmp_opcode(reg, memory, op, addr);
  res += cpx_opcode(reg, memory, op, addr);
  res += cpy_opcode(reg, memory, op, addr);
  res += dec_opcode(reg, memory, op, addr);
  res += inc_opcode(reg, memory, op, addr);
  res += dex_opcode(reg, memory, op);
  res += dey_opcode(reg, memory, op);
  res += inx_opcode(reg, memory, op);
  res += iny_opcode(reg, memory, op);
  res += eor_opcode(reg, memory, op, addr);
  res += jmp_opcode(reg, memory, op, addr);
  res += jsr_opcode(reg, memory, op, addr);
  res += lda_opcode(reg, memory, op, addr);
  res += ldx_opcode(reg, memory, op, addr);
  res += ldy_opcode(reg, memory, op, addr);
  res += lsr_opcode(reg, memory, op, addr);
  res += ora_opcode(reg, memory, op, addr);
  res += sta_opcode(reg, memory, op, addr);
  res += stx_opcode(reg, memory, op, addr);
  res += sty_opcode(reg, memory, op, addr);
  res += rol_opcode(reg, memory, op, addr);
  res += ror_opcode(reg, memory, op, addr);
  res += rts_opcode(reg, memory, op, addr);
  res += psp_opcode(reg, memory, op);
  res += sbc_opcode(reg, memory, op, addr);
  res += nop_opcode(reg, memory, op);
  assert(res < 2);
  if (res) {
    // found
  } else {
    debug("OP=%x not found", op);
    reg->pc++;
  }

  print_register(reg);
  // debug("cycle=%d", cycle);

  return 0;
}

int cpu_run(void* unused) {

  int debug = 0;
  int brk = 0x0724;
  uint64_t t = 1000 * 1000 / CPU_FREQ; // tick every 1us // limit to 1Mhz
  int quit = 0;
  while (!quit) {

    // TODO: create a dedicated debugger function
    // And replace debug log with the name of instruction and value
    if (__cpu_reg.pc == brk) debug = 1;
    if (debug) {
      int c = getchar();
      if (c == 'p') {
        // hexdumpSnake(*(__memory + 0x200), 1024);
        continue;
      } else if (c == 'r') {
        debug = 0;
        continue;
      }
      // lf 10
      quit = cpu_exec(__memory, &__cpu_reg);
      continue;
    }

    const struct timespec time = {.tv_sec = CPU_FREQ == 1 ? 1 : 0, .tv_nsec = CPU_FREQ == 1 ? 0 : 1000 * t};
    const int sleep = nanosleep(&time, NULL);
    quit = cpu_exec(__memory, &__cpu_reg);
  }
  
  return quit;
}

// used in test
void run(t_mem *memory, size_t size, t_registers *reg) {

  debug("RUN 6502 with a memory of %zu octets", size);

  while (cpu_exec(memory, reg) != 0) {

  }
  
}