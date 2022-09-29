#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "debug.h"
#include "bus.h"

// todo:  global variable
// handle State Machine with cycle
uint32_t cycle = 0; 

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

void check_processor_status(int32_t lastValue, int8_t value, t_registers *reg)
{

  // TODO : https://www.doc.ic.ac.uk/~eedwards/compsys/arithmetic/index.html#:~:text=Overflow%20Rule%20for%20addition,result%20has%20the%20opposite%20sign.
  // carry should be checked with the sign of the bit 7 and not from a cast to 32bit variable
  reg->p.C = (lastValue > 127 || lastValue < -128) ? 1 : 0; // carry
  reg->p.Z = value == 0 ? 1 : 0; // zero
  reg->p.N = value < 0 ? 1 : 0; // neg
  reg->p.V = lastValue > value ? 1 : 0; // overflow

  // TODO:
  // Refactor 
  // see sbc_opcode
}

uint8_t getAddressMode(t_e_mode mode, union u16 arg, t_registers *reg, t_mem *memory) {

  VB4(printf("addressMode arg(LSB=%x,MSB=%x,v=%x)", arg.lsb, arg.msb, arg.value));

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
  }
  return 0;
}

uint8_t addressMode(t_e_mode mode, union u16 arg, t_registers *reg, t_mem *memory) {

  return readbus(memory, getAddressMode(mode, arg, reg, memory));
}

int adc_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 addr) {
  int16_t lastValue;

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

int and_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  switch (op)
  {
  case 0x29:
    VB2(printf("and opcode immediate"));
    reg->a &= readbus(memory, reg->pc + 1); // or arg.lsb

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

int asl_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  int32_t last_value;
  switch (op)
  {
  case 0x0a:
    VB2(printf("asl opcode accumulator"));
    last_value = reg->a << 1;
    reg->a <<= 1;

    cycle += 2;
    reg->pc += 1;
    check_processor_status(last_value, reg->a, reg);
    break;
  
  case 0x06:
    VB2(printf("asl opcode zeropage"));
    last_value = readbus(memory, getAddressMode(zero_page, arg, reg, memory)) << 1;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), last_value);

    cycle += 5;
    reg->pc += 2;
    // TODO
    check_processor_status(last_value, *memory[arg.lsb], reg);
    break;

  case 0x16:
    VB2(printf("asl opcode zeropagex"));
    last_value = readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) << 1;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), last_value);

    cycle += 6;
    reg->pc += 2;
    // TODO
    check_processor_status(last_value, *memory[arg.lsb + reg->x], reg);
    break;

  case 0x0e:
    VB2(printf("asl opcode absolute"));
    last_value = readbus(memory, getAddressMode(absolute, arg, reg, memory)) << 1;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), last_value);

    cycle += 6;
    reg->pc += 3;
    // TODO
    check_processor_status(last_value, *memory[arg.value], reg);
    break;

  case 0x1e:
    VB2(printf("asl opcode absolutex"));
    last_value = readbus(memory, getAddressMode(absolute_x, arg, reg, memory)) << 1;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), last_value);

    cycle += 7;
    reg->pc += 3;
    // TODO
    check_processor_status(last_value, *memory[arg.value + reg->x], reg);
    break;

  default:
    return 0;
  }

  return 1;
}

int bcc_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0x90) {
    VB2(printf("bcc opcode relative"));

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
    VB2(printf("bcs opcode relative"));

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
    VB2(printf("beq opcode relative"));

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

  int8_t value;
  switch (op)
  {
  case 0x24:
    value = reg->a & readbus(memory, arg.lsb);
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
  reg->p.Z = (value == 0) ? 1 : 0;
  reg->p.V = (value & 0b100000) ? 1 : 0;
  reg->p.N = (value & 0b1000000) ? 1 : 0;

  return 1;
}

int bmi_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0x30) {
    VB2(printf("bmi opcode relative"));

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
    VB2(printf("bne opcode relative"));

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
    VB2(printf("bpl opcode relative"));

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
    VB2(printf("bvc opcode relative"));

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
    VB2(printf("bvs opcode relative"));

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
    VB2(printf("clc opcode implied"));
    // clc
    reg->p.C = 0;
    break;
  
  case 0xd8:
    VB2(printf("cld opcode implied"));
    // cld
    reg->p.D = 0;
    break;

  case 0x58:
    VB2(printf("cli opcode implied"));
    // cli
    reg->p.I = 0;
    break;

  case 0xb8:
    VB2(printf("clv opcode implied"));
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

int set_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  switch (op)
  {
  case 0x38:
    VB2(printf("sec opcode implied"));
    // sec
    reg->p.C = 1;
    break;
  
  case 0xf8:
    // sed
    VB2(printf("sed opcode implied"));
    reg->p.D = 1;
    break;

  case 0x78:
    // sei
    VB2(printf("sei opcode implied"));
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

  switch (op)
  {
  case 0xaa:
    VB2(printf("tax opcode implied"));
    // tax
    reg->x = reg->a;
    check_processor_status(reg->x, reg->x, reg);
    break;
  
  case 0xa8:
    VB2(printf("tay opcode implied"));
    // tay
    reg->y = reg->a;
    check_processor_status(reg->y, reg->y, reg);
    break;

  case 0xba:
    VB2(printf("tsx opcode implied"));
    // tsx
    reg->x = reg->sp;
    check_processor_status(reg->x, reg->x, reg);
    break;

  case 0x8a:
    VB2(printf("txa opcode implied"));
    // txa
    reg->a = reg->x;
    check_processor_status(reg->a, reg->a, reg);
    break;
  
  case 0x9a:
    VB2(printf("txs opcode implied"));
    // txs
    reg->a = reg->sp;
    // no check status
    break;
  
  case 0x98:
    VB2(printf("tya opcode implied"));
    // tya
    reg->a = reg->y;
    check_processor_status(reg->a, reg->a, reg);
    break;

  default:
    return 0;
  }

  reg->pc += 1;
  cycle += 2;
  return 1;
}

int cmp_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  int32_t last_value;
  switch (op)
  {
  case 0xc9:
    VB2(printf("cmp opcode immediate"));
    last_value = reg->a - arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xc5:
    VB2(printf("cmp opcode zeropage"));
    last_value = reg->a - addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xd5:
    VB2(printf("cmp opcode zeropagex"));
    last_value = reg->a - addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xcd:
    VB2(printf("cmp opcode absolute"));
    last_value = reg->a - addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xdd:
    VB2(printf("cmp opcode absolutex"));
    last_value = reg->a - addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  case 0xd9:
    VB2(printf("cmp opcode absolutey"));
    last_value = reg->a - addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;
  
  case 0xc1:
    VB2(printf("cmp opcode indirectx"));
    last_value = reg->a - addressMode(indirect_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 6;
    break;

  case 0xd1:
    VB2(printf("cmp opcode indirecty"));
    last_value = reg->a - addressMode(indirect_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 5; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = last_value == 0 ? 1 : 0;
  reg->p.C = last_value > 0 ? 1 : 0;
  reg->p.N = last_value < 0 ? 1 : 0;
  
  return 1;
}

int cpx_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  int32_t last_value;
  switch (op)
  {
  case 0xe0:
    VB2(printf("cpx opcode immediate"));
    last_value = reg->x - arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xe4:
    VB2(printf("cpx opcode zeropage"));
    last_value = reg->x - addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xec:
    VB2(printf("cpx opcode absolute"));
    last_value = reg->x - addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  default:
    return 0;
  }

  reg->p.Z = last_value == 0 ? 1 : 0;
  reg->p.C = last_value > 0 ? 1 : 0;
  reg->p.N = last_value < 0 ? 1 : 0;

  return 1;
}

int cpy_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  int32_t last_value;
  switch (op)
  {
  case 0xc0:
    VB2(printf("cpy opcode immediate"));
    last_value = reg->y - arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xc4:
    VB2(printf("cpy opcode zeropage"));
    last_value = reg->y - addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xcc:
    VB2(printf("cpy opcode absolute"));
    last_value = reg->y - addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  default:
    return 0;
  }

  reg->p.Z = last_value == 0 ? 1 : 0;
  reg->p.C = last_value > 0 ? 1 : 0;
  reg->p.N = last_value < 0 ? 1 : 0;
  
  return 1;
}

int dec_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  int32_t last_value;
  switch (op)
  {
  case 0xc6:
    VB2(printf("dec opcode zeropage"));
    last_value = readbus(memory, getAddressMode(zero_page, arg, reg, memory)) - 1;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), last_value);

    reg->pc += 2;
    cycle += 5;
    break;
  
  case 0xd6:
    VB2(printf("dec opcode zeropagex"));
    last_value = readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) - 1;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), last_value);

    reg->pc += 2;
    cycle += 6;

  case 0xce:
    VB2(printf("dec opcode absolute"));
    last_value = readbus(memory, getAddressMode(absolute, arg, reg, memory)) - 1;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), last_value);

    reg->pc += 3;
    cycle += 6;
  
  case 0xde:
    VB2(printf("dec opcode absolutex"));
    last_value = readbus(memory, getAddressMode(absolute_x, arg, reg, memory)) - 1;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), last_value);

    reg->pc += 3;
    cycle += 7;
  

  default:
    return 0;
  }

  reg->p.Z = last_value == 0 ? 1 : 0;
  reg->p.N = last_value < 0 ? 1 : 0;

  return 1;
}

int inc_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  int32_t last_value;
  switch (op)
  {
  case 0xe6:
    VB2(printf("inc opcode zeropage"));
    last_value = readbus(memory, getAddressMode(zero_page, arg, reg, memory)) + 1;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), last_value);

    reg->pc += 2;
    cycle += 5;
    break;
  
  case 0xf6:
    VB2(printf("inc opcode zeropagex"));
    last_value = readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) + 1;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), last_value);

    reg->pc += 2;
    cycle += 6;

  case 0xee:
    VB2(printf("inc opcode absolute"));
    last_value = readbus(memory, getAddressMode(absolute, arg, reg, memory)) + 1;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), last_value);

    reg->pc += 3;
    cycle += 6;
  
  case 0xfe:
    VB2(printf("inc opcode absolutex"));
    last_value = readbus(memory, getAddressMode(absolute_x, arg, reg, memory)) + 1;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), last_value);

    reg->pc += 3;
    cycle += 7;
  

  default:
    return 0;
  }

  reg->p.Z = last_value == 0 ? 1 : 0;
  reg->p.N = last_value < 0 ? 1 : 0;
  
  return 1;
}

int dex_opcode(t_registers *reg, t_mem *memory, uint8_t op) {

  if (op == 0xca)
  {
    VB2(printf("dex opcode implied"));
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
    VB2(printf("dey opcode implied"));
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
    VB2(printf("inx opcode implied"));
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
    VB2(printf("iny opcode implied"));
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

  int32_t last_value;
  switch (op)
  {
  case 0x49:
    VB2(printf("eor opcode immediate"));
    last_value = reg->a ^ arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0x45:
    VB2(printf("eor opcode zeropage"));
    last_value = reg->a ^ addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0x55:
    VB2(printf("eor opcode zeropagex"));
    last_value = reg->a ^ addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0x4d:
    VB2(printf("eor opcode absolute"));
    last_value = reg->a ^ addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0x5d:
    VB2(printf("eor opcode absolutex"));
    last_value = reg->a ^ addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  case 0x59:
    VB2(printf("eor opcode absolutey"));
    last_value = reg->a ^ addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;
  
  case 0x41:
    VB2(printf("eor opcode indirectx"));
    last_value = reg->a ^ addressMode(indirect_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 6;
    break;

  case 0x51:
    VB2(printf("eor opcode indirecty"));
    last_value = reg->a ^ addressMode(indirect_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 5; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = last_value == 0 ? 1 : 0;
  reg->p.N = last_value < 0 ? 1 : 0;

  return 1;
}

int jmp_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  int32_t last_value;
  switch (op)
  {
  case 0x4c:
    VB2(printf("jmp opcode immediate"));
    reg->pc += arg.value; // TODO/QUESTION jump is after opcode jmp or before ?
    cycle += 3;
    break;
  
  case 0x6c:
    VB2(printf("jmp opcode indirect"));
    VB2(printf("TODO !! need to implement it"));
    reg->pc += 3; // TODO: how to do indirect jump ? https://www.nesdev.org/obelisk-6502-guide/addressing.html#IND
    cycle += 5;
    break;
  
  default:
    return 0;
  }
  
  return 1;
}

int lda_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  switch (op)
  {
  case 0xa9:
    VB2(printf("lda opcode immediate"));
    reg->a = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xa5:
    VB2(printf("lda opcode zeropage"));
    reg->a = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xb5:
    VB2(printf("lda opcode zeropagex"));
    reg->a = addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xad:
    VB2(printf("lda opcode absolute"));
    reg->a = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xbd:
    VB2(printf("lda opcode absolutex"));
    reg->a = addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  case 0xb9:
    VB2(printf("lda opcode absolutey"));
    reg->a = addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;
  
  case 0xa1:
    VB2(printf("lda opcode indirectx"));
    reg->a = addressMode(indirect_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 6;
    break;

  case 0xb1:
    VB2(printf("lda opcode indirecty"));
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

  switch (op)
  {
  case 0xa2:
    VB2(printf("ldx opcode immediate"));
    reg->x = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xa6:
    VB2(printf("ldx opcode zeropage"));
    reg->x = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xb6:
    VB2(printf("ldx opcode zeropagey"));
    reg->x = addressMode(zero_page_y, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xae:
    VB2(printf("ldx opcode absolute"));
    reg->x = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xbe:
    VB2(printf("ldx opcode absolutey"));
    reg->x = addressMode(absolute_y, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = reg->a == 0 ? 1 : 0;
  reg->p.N = reg->a < 0 ? 1 : 0;

  return 1;
}

int ldy_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  switch (op)
  {
  case 0xa0:
    VB2(printf("ldy opcode immediate"));
    reg->a = arg.lsb;

    reg->pc += 2;
    cycle += 2;
    break;
  
  case 0xa4:
    VB2(printf("ldy opcode zeropage"));
    reg->y = addressMode(zero_page, arg, reg, memory);

    reg->pc += 2;
    cycle += 3;
    break;
  
  case 0xb4:
    VB2(printf("ldy opcode zeropagex"));
    reg->y = addressMode(zero_page_x, arg, reg, memory);

    reg->pc += 2;
    cycle += 4;
    break;

  case 0xac:
    VB2(printf("ldy opcode absolute"));
    reg->y = addressMode(absolute, arg, reg, memory);

    reg->pc += 3;
    cycle += 4;
    break;

  case 0xbc:
    VB2(printf("ldy opcode absolutex"));
    reg->y = addressMode(absolute_x, arg, reg, memory);

    reg->pc += 3;
    cycle += 4; // TODO +1 if page crossed
    break;

  default:
    return 0;
  }

  reg->p.Z = reg->a == 0 ? 1 : 0;
  reg->p.N = reg->a < 0 ? 1 : 0;

  return 1;
}

int lsr_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  int32_t last_value;
  switch (op)
  {
  case 0x4a:
    VB2(printf("lsr opcode immediate"));
    last_value = reg->a >> 1;
    reg->a >>= 1;

    cycle += 2;
    reg->pc += 1;
    check_processor_status(last_value, reg->a, reg);
    break;
  
  case 0x46:
    VB2(printf("lsr opcode zeropage"));
    last_value = addressMode(zero_page, arg, reg, memory) >> 1;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), last_value);

    cycle += 5;
    reg->pc += 2;
    // TODO
    check_processor_status(last_value, *memory[arg.lsb], reg);
    break;

  case 0x56:
    VB2(printf("lsr opcode zeropagex"));
    last_value = addressMode(zero_page_x, arg, reg, memory) >> 1;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), last_value);

    cycle += 6;
    reg->pc += 2;
    // TODO
    check_processor_status(last_value, *memory[arg.lsb + reg->x], reg);
    break;

  case 0x4e:
    VB2(printf("lsr opcode absolute"));
    last_value = addressMode(absolute, arg, reg, memory) >> 1;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), last_value);

    cycle += 6;
    reg->pc += 3;
    // TODO
    check_processor_status(last_value, *memory[arg.value], reg);
    break;

  case 0x5e:
    VB2(printf("lsr opcode absolutex"));
    last_value = addressMode(absolute_x, arg, reg, memory) >> 1;
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), last_value);

    cycle += 7;
    reg->pc += 3;
    // TODO
    check_processor_status(last_value, *memory[arg.value + reg->x], reg);
    break;

  default:
    return 0;
  }

  return 1;
}

int ora_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  switch (op)
  {
  case 0x09:
    VB2(printf("ora opcode immediate"));
    reg->a |= readbus(memory, reg->pc + 1);

    cycle += 2;
    reg->pc += 2;
    break;
  case 0x05:
    VB2(printf("ora opcode zeropage"));
    reg->a |= addressMode(zero_page, arg, reg, memory);

    cycle += 3;
    reg->pc += 2;
    break;
  
  case 0x15:
    VB2(printf("ora opcode zeropagex"));
    reg->a |= addressMode(zero_page_x, arg, reg, memory);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x0d:
    VB2(printf("ora opcode absolute"));
    reg->a &= addressMode(absolute, arg, reg, memory);

    cycle += 4;
    reg->pc += 3;
    break;
  
  case 0x1d:
    VB2(printf("ora opcode absolutex"));
    reg->a &= addressMode(absolute_x, arg, reg, memory);

    cycle += 4; // Todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x19:
    VB2(printf("ora opcode absolutey"));
    reg->a |= addressMode(absolute_y, arg, reg, memory);

    cycle += 4; // todo +1 if page crossed
    reg->pc += 3;
    break;
  
  case 0x01:
    VB2(printf("ora opcode indirectx"));
    reg->a |= addressMode(indirect_x, arg, reg, memory);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x11:
    VB2(printf("ora opcode indirecty"));
    reg->a |= addressMode(indirect_y, arg, reg, memory);

    cycle += 5; // todo +1 if page crossed
    reg->pc += 2;
    break;
  
  default:
    return 0;
  }

  check_processor_status(reg->a, reg->a, reg);
  return 1;
}

int sta_opcode(t_registers *reg, t_mem *memory, uint8_t op, union u16 arg) {

  switch (op)
  {
  case 0x85:
    VB2(printf("sta opcode zeropage"));
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), reg->a);

    cycle += 3;
    reg->pc += 2;
    break;
  case 0x95:
    VB2(printf("sta opcode zeropagex"));
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), reg->a);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x8d:
    VB2(printf("sta opcode absolute"));
    writebus(memory, getAddressMode(absolute, arg, reg, memory), reg->a);

    cycle += 4;
    reg->pc += 3;
    break;
  
  case 0x9d:
    VB2(printf("sta opcode absolutex"));
    writebus(memory, getAddressMode(absolute_x, arg, reg, memory), reg->a);

    cycle += 5;
    reg->pc += 3;
    break;

  case 0x99:
    VB2(printf("sta opcode absolutey"));
    writebus(memory, getAddressMode(absolute_y, arg, reg, memory), reg->a);

    cycle += 5;
    reg->pc += 3;
    break;

  case 0x81:
    VB2(printf("sta opcode indirectx"));
    writebus(memory, getAddressMode(indirect_x, arg, reg, memory), reg->a);

    cycle += 6;
    reg->pc += 2;
    break;

  case 0x91:
    VB2(printf("sta opcode indirecty"));
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

  switch (op)
  {
  case 0x86:
    VB2(printf("stx opcode zeropage"));
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), reg->x);

    cycle += 3;
    reg->pc += 2;
    break;
  case 0x96:
    VB2(printf("stx opcode zeropageY"));
    writebus(memory, getAddressMode(zero_page_y, arg, reg, memory), reg->x);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x8e:
    VB2(printf("stx opcode absolute"));
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

  switch (op)
  {
  case 0x84:
    VB2(printf("sty opcode zeropage"));
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), reg->y);

    cycle += 3;
    reg->pc += 2;
    break;

  case 0x94:
    VB2(printf("sty opcode zeropageX"));
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), reg->y);

    cycle += 4;
    reg->pc += 2;
    break;
  
  case 0x8c:
    VB2(printf("sty opcode absolute"));
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

  int last_value, oldbit = 0;
  switch (op)
  {
  case 0x2a:
    VB2(printf("rol opcode immediate"));
    oldbit = (reg->a & 0x80) >> 7;
    reg->a <<= 1;
    reg->a = reg->a | reg->p.C;
    reg->p.C = oldbit;

    reg->pc += 1;
    cycle += 2;
    break;

  case 0x26:
    VB2(printf("rol opcode zeropage"));
    oldbit = (addressMode(zero_page, arg, reg, memory) & 0x80) >> 7;
    reg->a <<= 1;
    last_value = readbus(memory, getAddressMode(zero_page, arg, reg, memory)) | reg->p.C;
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 2;
    cycle += 5;
    break;

  case 0x36:
    VB2(printf("rol opcode zeropagex"));
    oldbit = (addressMode(zero_page_x, arg, reg, memory) & 0x80) >> 7;
    reg->a <<= 1;
    last_value = readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) | reg->p.C;
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 2;
    cycle += 6;
    break;

  case 0x2e:
    VB2(printf("rol opcode absolute"));
    oldbit = (addressMode(absolute, arg, reg, memory) & 0x80) >> 7;
    reg->a <<= 1;
    last_value = readbus(memory, getAddressMode(absolute, arg, reg, memory)) | reg->p.C;
    writebus(memory, getAddressMode(absolute, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 3;
    cycle += 6;
    break;
 
  case 0x3e:
    VB2(printf("rol opcode absolutex"));
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

  int last_value, oldbit = 0;
  switch (op)
  {
  case 0x6a:
    VB2(printf("ror opcode immediate"));
    oldbit = reg->a & 0x01;
    reg->a >>= 1;
    last_value = reg->a |= reg->p.C << 7;
    reg->p.C = oldbit;

    reg->pc += 1;
    cycle += 2;
    break;

  case 0x66:
    VB2(printf("ror opcode zeropage"));
    oldbit = addressMode(zero_page, arg, reg, memory) & 0x01;
    reg->a >>= 1;
    last_value = readbus(memory, getAddressMode(zero_page, arg, reg, memory)) | (reg->p.C << 7);
    writebus(memory, getAddressMode(zero_page, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 2;
    cycle += 5;
    break;

  case 0x76:
    VB2(printf("ror opcode zeropagex"));
    oldbit = addressMode(zero_page_x, arg, reg, memory) & 0x01;
    reg->a >>= 1;
    last_value = readbus(memory, getAddressMode(zero_page_x, arg, reg, memory)) | (reg->p.C << 7);
    writebus(memory, getAddressMode(zero_page_x, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 2;
    cycle += 6;
    break;

  case 0x6e:
    VB2(printf("ror opcode absolute"));
    oldbit = addressMode(absolute, arg, reg, memory) & 0x01;
    reg->a >>= 1;
    last_value = readbus(memory, getAddressMode(absolute, arg, reg, memory)) | (reg->p.C << 7);
    writebus(memory, getAddressMode(absolute, arg, reg, memory), last_value);
    reg->p.C = oldbit;

    reg->pc += 3;
    cycle += 6;
    break;
 
  case 0x7e:
    VB2(printf("ror opcode absolutex"));
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
    VB2(printf("jsr opcode absolute"));
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
    VB2(printf("rts opcode implied"));
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
    VB2(printf("pha opcode implied"));
    writebus(memory, 0x01ff - reg->sp, reg->a);
    reg->sp--;
    reg->pc += 1;
    cycle += 3;
    break;
  
  case 0x08:
    VB2(printf("php opcode implied"));
    writebus(memory, 0x01ff - reg->sp, reg->p.value);
    reg->sp--;
    reg->pc += 1;
    cycle += 3;
    break;

  case 0x68:
    VB2(printf("pla opcode implied"));
    reg->sp++;
    reg->a = readbus(memory, 0x01ff - reg->sp);
    check_processor_status(reg->a, reg->a, reg);
    reg->pc += 1;
    cycle += 3;
    break;

  case 0x28:
    VB2(printf("plp opcode implied"));
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
    VB2(printf("brk opcode"));
    irq(reg, memory);
    reg->p.B = 1;

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

int exec(t_mem *memory, t_registers *reg) {

  int res = 0;
  uint8_t op = readbus(memory, reg->pc);
  union u16 addr = readbus16(memory, reg->pc + 1);
#ifdef DEBUG_CPU
  if (op == 0)
  {
    // break;
    VB1(printf("DEBUG_CPU BREAK"));
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
  assert(res < 2);
  if (res) {
    // found
  } else {
    VB2(printf("OP=%x not found", op));
    reg->pc++;
  }

  VB4(print_register(reg));
  VB4(printf("cycle=%d", cycle));

  if (op == 0xf0) printf("--------\n");

  return 0;
}

void run(t_mem *memory, size_t size, t_registers *reg) {

  VB0(printf("RUN 6502 with a memory of %zu octets", size));

  while (exec(memory, reg) != 0) {

  }
  
}