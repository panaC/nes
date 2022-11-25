#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cpu.h"

#ifndef DEBUG_CPU
#  define DEBUG_CPU 1
#endif

#ifndef DEBUG_CPU
#  define debug(...) 0;
#else
#  define debug_start() fprintf(stdout, "CPU: ");
#  define debug_content(...) fprintf(stdout, __VA_ARGS__);
#  define debug_end() fprintf(stdout, "\n");
#  define debug(...) debug_start();debug_content(__VA_ARGS__);debug_end();
#endif

// extern CPU Variable
cpu_readbus_t *cpu_readbus = NULL;
cpu_readbus16_t *cpu_readbus16 = NULL;
cpu_writebus_t *cpu_writebus = NULL;

// local read/write bus variable function
// init in cpu_init with assert on NULL;
cpu_readbus_t *readbus = NULL;
cpu_writebus_t *writebus = NULL;
cpu_readbus16_t *readbus16 = NULL;


/**
 * 6502 it vector
 *
 *
 * NMI        -> LSB:0xFFFA - MSB:0xFFFB
 * RESET      -> LSB:0xFFFC - MSB:0xFFFD
 * IRQ/BRK    -> LSB:0xFFFE - MSB:0xFFFF
 */

struct instruction _op[0x100] = { 0 };

t_registers __cpu_reg = {
  .pc = 0,
  .sp = 0,
  .p = 0,
  .a = 0,
  .x = 0,
  .y = 0 };

int __no_rw_debug = 0;
int __debug_brk = 0;
int __debug_assert_pc = 0;

static union u16 readbus16_default(uint32_t addr) {
  return (union u16) { .lsb = readbus(addr), .msb = readbus(addr + 1) };
}

static uint8_t readbus_pc() {
  return readbus(__cpu_reg.pc + 1);
}

static union u16 readbus16_pc() {
  return readbus16(__cpu_reg.pc + 1);
}

static void debug_opcode(enum e_addressMode mode, char* str, uint8_t op, union u16 arg) {

  // debug_content("$%04x ", __cpu_reg.pc);
  debug_content("a=%02x  ", __cpu_reg.a & 0xff);
  debug_content("x=%02x  ", __cpu_reg.x & 0xff);
  debug_content("y=%02x  ", __cpu_reg.y & 0xff);
  debug_content("sp=%02x  ", __cpu_reg.sp & 0xff);
  debug_content("%c", __cpu_reg.p.N ? 'N' : '-');
  debug_content("%c", __cpu_reg.p.V ? 'V' : '-');
  debug_content("-");
  debug_content("%c", __cpu_reg.p.B ? 'B' : '-');
  debug_content("%c", __cpu_reg.p.D ? 'D' : '-');
  debug_content("%c", __cpu_reg.p.I ? 'I' : '-');
  debug_content("%c", __cpu_reg.p.Z ? 'Z' : '-');
  debug_content("%c", __cpu_reg.p.C ? 'C' : '-');
  debug_content("\t  ");

  switch (mode)
  {
    case ACCUMULATOR:
      debug_content("$%04x    %02x           %s\t\t| ", __cpu_reg.pc, op, str);
      break;
    case IMPLIED:
      debug_content("$%04x    %02x           %s\t\t| ", __cpu_reg.pc, op, str);
      break;
    case IMMEDIATE:
      debug_content("$%04x    %02x %02x        %s #$%02x\t| ", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
      break;
    case ABSOLUTE:
      debug_content("$%04x    %02x %02x %02x     %s $%04x\t| ", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
      break;
    case ZEROPAGE:
      debug_content("$%04x    %02x %02x        %s $%02x\t\t| ", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
      break;
    case RELATIVE:
      debug_content("$%04x    %02x %02x        %s $%02x\t\t| ", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
      break;
    case ABSOLUTEX:
      debug_content("$%04x    %02x %02x %02x     %s $%04x,X\t| ", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
      break;
    case ABSOLUTEY:
      debug_content("$%04x    %02x %02x %02x     %s $%04x,Y\t| ", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
      break;
    case ZEROPAGEX:
      debug_content("$%04x    %02x %02x        %s $%02x,X\t| ", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
      break;
    case ZEROPAGEY:
      debug_content("$%04x    %02x %02x        %s $%02x,Y\t| ", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
      break;
    case INDIRECT:
      debug_content("$%04x    %02x %02x %02x     %s ($%04x)\t| ", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
      break;
    case INDIRECTX:
      debug_content("$%04x    %02x %02x        %s ($%02x,X)\t| ", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
      break;
    case INDIRECTY:
      debug_content("$%04x    %02x %02x        %s ($%02x,Y)\t| ", __cpu_reg.pc, op, arg.lsb, str, arg.lsb);
      break;

    default:
      break;
  }

}

static uint32_t addressmode(enum e_addressMode mode, union u16 arg) {

  switch (mode)
  {
    case ZEROPAGE:
      return arg.lsb;
    case ZEROPAGEX:
      return (arg.lsb + __cpu_reg.x) % 256;
    case ZEROPAGEY:
      return (arg.lsb + __cpu_reg.y) % 256;
    case ABSOLUTE:
      return arg.value;
    case ABSOLUTEX:
      return arg.value + __cpu_reg.x;
    case ABSOLUTEY:
      return arg.value + __cpu_reg.y;
    case INDIRECTX:
      return readbus((arg.lsb + __cpu_reg.x) % 256) + readbus((arg.lsb + __cpu_reg.x + 1) % 256) * 256;
    case INDIRECTY:
      return readbus(arg.lsb) + readbus((arg.lsb + 1) % 256) * 256 + __cpu_reg.y;

    case ACCUMULATOR:
    case IMPLIED:
    case IMMEDIATE:
    case INDIRECT:
    case RELATIVE:
      break;
  }

  assert(0);
  return 0;
}

static uint8_t readDataFromAddressmode(enum e_addressMode mode, union u16 arg) {
  if (mode == IMMEDIATE) {
    return arg.lsb;
  }
  return readbus(addressmode(mode, arg));
}

static union u16 read_arg(enum e_addressMode mode) {

  switch (mode)
  {
    case ACCUMULATOR:
      return (union u16) { .value = 0 };
    case IMPLIED:
      return (union u16) { .value = 0 };
    case IMMEDIATE:
      return (union u16) { .value = readbus_pc() };
    case ABSOLUTE:
      return readbus16_pc();
    case ZEROPAGE:
      return (union u16) { .value = readbus_pc() };
    case RELATIVE:
      return (union u16) { .value = readbus_pc() };
    case ABSOLUTEX:
      return readbus16_pc();
    case ABSOLUTEY:
      return readbus16_pc();
    case ZEROPAGEX:
      return (union u16) { .value = readbus_pc() };
    case ZEROPAGEY:
      return (union u16) { .value = readbus_pc() };
    case INDIRECT:
      return readbus16_pc();
    case INDIRECTX:
      return (union u16) { .value = readbus_pc() };
    case INDIRECTY:
      return (union u16) { .value = readbus_pc() };
  }

  return (union u16) { .value = 0 };
}

static int16_t not_found() {

  debug("OP=%x not found", readbus(__cpu_reg.pc));
  assert(0);

  return 0;
}

static int16_t nop() {

  // nop

  return 0;
}

static int16_t brk() {
  // irq();
  __cpu_reg.p.B = 1;

  return 0;
}

static void sp_zn(int16_t result)
{
  __cpu_reg.p.Z = !result;
  __cpu_reg.p.N = !!(result & 0x80);
}

static int16_t adc(enum e_addressMode mode, union u16 uarg) {

  uint8_t mem = readDataFromAddressmode(mode, uarg);
  int16_t result = __cpu_reg.a += mem + __cpu_reg.p.C;

  // http://www.6502.org/tutorials/vflag.html
  // seems to be the right value : see in github nes emulation
  __cpu_reg.p.C = !!(result & 0x100);
  // As stated above, the second purpose of the carry flag
  // is to indicate when the result of the
  // addition or subtraction is outside the range 0 to 255, specifically:
  // When the addition result is 0 to 255, the carry is cleared.
  // When the addition result is greater than 255, the carry is set.

  // http://www.6502.org/tutorials/vflag.html
  __cpu_reg.p.V = result >= 0 ? (result & 0x100) >> 8 : !((result & 0x80) >> 7);

  return result & 0xff; // result may be equal to 0x100 with the carry
}

static int16_t and (enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.a &= readDataFromAddressmode(mode, uarg);
}

static int16_t asl(enum e_addressMode mode, union u16 uarg) {
  uint16_t result = 0;

  if (mode == ACCUMULATOR) {
    result = __cpu_reg.a << 1;
  } else {
    uint32_t addr = addressmode(mode, uarg);
    result = readbus(addr) << 1;
    writebus(addr, result);
  }
  __cpu_reg.a = result;
  __cpu_reg.p.C = !!(result & 0x80); // Set to contents of old bit 7
  return result;
}

static int16_t lsr(enum e_addressMode mode, union u16 uarg) {
  uint16_t result = 0;
  uint8_t mem = 0;

  if (mode == ACCUMULATOR) {
    mem = __cpu_reg.a;
    result = __cpu_reg.a >> 1;
  } else {
    uint32_t addr = addressmode(mode, uarg);
    mem = readbus(addr);
    result = mem >> 1;
    writebus(addr, result);
  }
  __cpu_reg.a = result;
  __cpu_reg.p.C = mem & 0x01; // Set to contents of old bit 0
  return result;
}

static int16_t bcc(enum e_addressMode mode, union u16 uarg) {
  if (__cpu_reg.p.C == 0)
    __cpu_reg.pc += (int8_t)uarg.lsb;
  return 0;
}

static int16_t bcs(enum e_addressMode mode, union u16 uarg) {
  if (__cpu_reg.p.C == 1)
    __cpu_reg.pc += (int8_t)uarg.lsb;
  return 0;
}

static int16_t beq(enum e_addressMode mode, union u16 uarg) {
  if (__cpu_reg.p.Z == 1)
    __cpu_reg.pc += (int8_t)uarg.lsb;
  return 0;
}

static int16_t bmi(enum e_addressMode mode, union u16 uarg) {
  if (__cpu_reg.p.N == 1)
    __cpu_reg.pc += (int8_t)uarg.lsb;
  return 0;
}

static int16_t bne(enum e_addressMode mode, union u16 uarg) {
  if (__cpu_reg.p.Z == 0)
    __cpu_reg.pc += (int8_t)uarg.lsb;
  return 0;
}

static int16_t bpl(enum e_addressMode mode, union u16 uarg) {
  if (__cpu_reg.p.N == 0)
    __cpu_reg.pc += (int8_t)uarg.lsb;
  return 0;
}

static int16_t bvc(enum e_addressMode mode, union u16 uarg) {
  if (__cpu_reg.p.V == 0)
    __cpu_reg.pc += (int8_t)uarg.lsb;
  return 0;
}

static int16_t bvs(enum e_addressMode mode, union u16 uarg) {
  if (__cpu_reg.p.V == 1)
    __cpu_reg.pc += (int8_t)uarg.lsb;
  return 0;
}

static int16_t bit(enum e_addressMode mode, union u16 uarg) {

  uint8_t mem = readbus(addressmode(mode, uarg));
  int8_t value = __cpu_reg.a & mem;

  __cpu_reg.p.Z = !value;
  __cpu_reg.p.V = !!(mem & 0x40);
  __cpu_reg.p.N = !!(mem & 0x80);

  return 0;
}

static int16_t clc(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.p.C = 0;
}

static int16_t cld(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.p.D = 0;
}

static int16_t cli(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.p.I = 0;
}

static int16_t clv(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.p.V = 0;
}

static int16_t sbc(enum e_addressMode mode, union u16 uarg) {
  int16_t result = __cpu_reg.a -= readDataFromAddressmode(mode, uarg) - (1 - __cpu_reg.p.C);

  // http://www.6502.org/tutorials/vflag.html
  __cpu_reg.p.C = !(!!(result & 0x100));
  // As stated above, the second purpose of the carry flag
  // is to indicate when the result of the
  // addition or subtraction is outside the range 0 to 255, specifically:
  // When the subtraction result is 0 to 255, the carry is set.
  // When the subtraction result is less than 0, the carry is cleared.

  // http://www.6502.org/tutorials/vflag.html
  __cpu_reg.p.V = result >= 0 ? (result & 0x100) >> 8 : !((result & 0x80) >> 7);

  return result;
}

static int16_t sec(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.p.C = 1;
}

static int16_t sed(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.p.D = 1;
}

static int16_t sei(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.p.I = 1;
}

static int16_t tax(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.x = __cpu_reg.a;
}

static int16_t tay(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.y = __cpu_reg.a;
}

static int16_t tsx(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.x = __cpu_reg.sp;
}

static int16_t txa(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.a = __cpu_reg.x;
}

static int16_t txs(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.sp = __cpu_reg.x;
}

static int16_t tya(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.a = __cpu_reg.y;
}

static int16_t cmp(enum e_addressMode mode, union u16 uarg) {
  int16_t result = __cpu_reg.a - readDataFromAddressmode(mode, uarg);
  __cpu_reg.p.C = !(!!(result & 0x80));
  return result;
}

static int16_t cpx(enum e_addressMode mode, union u16 uarg) {
  int16_t result = __cpu_reg.x - readDataFromAddressmode(mode, uarg);
  __cpu_reg.p.C = !(!!(result & 0x80));
  return result;
}

static int16_t cpy(enum e_addressMode mode, union u16 uarg) {
  int16_t result = __cpu_reg.y - readDataFromAddressmode(mode, uarg);
  __cpu_reg.p.C = !(!!(result & 0x80));
  return result;
}

static int16_t dec(enum e_addressMode mode, union u16 uarg) {
  uint32_t addr = addressmode(mode, uarg);
  uint8_t result = (int8_t)readbus(addr) - 1;
  writebus(addr, result);
  return result;
}

static int16_t inc(enum e_addressMode mode, union u16 uarg) {
  uint32_t addr = addressmode(mode, uarg);
  uint8_t result = (int8_t)readbus(addr) + 1;
  writebus(addr, result);
  return result;
}

static int16_t dex(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.x = (__cpu_reg.x - 1) & 0xff;
}

static int16_t dey(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.y = (__cpu_reg.y - 1) & 0xff;
}

static int16_t inx(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.x = (__cpu_reg.x + 1) & 0xff;
}

static int16_t iny(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.y = (__cpu_reg.y + 1) & 0xff;
}

static int16_t eor(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.a ^= (int8_t)readDataFromAddressmode(mode, uarg);
}

static int16_t lda(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.a = readDataFromAddressmode(mode, uarg);
}

static int16_t ldx(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.x = readDataFromAddressmode(mode, uarg);
}

static int16_t ldy(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.y = readDataFromAddressmode(mode, uarg);
}

static int16_t sta(enum e_addressMode mode, union u16 uarg) {
  writebus(addressmode(mode, uarg), __cpu_reg.a);
  return 0;
}

static int16_t stx(enum e_addressMode mode, union u16 uarg) {
  writebus(addressmode(mode, uarg), __cpu_reg.x);
  return 0;
}

static int16_t sty(enum e_addressMode mode, union u16 uarg) {
  writebus(addressmode(mode, uarg), __cpu_reg.y);
  return 0;
}

static int16_t ora(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.a |= readDataFromAddressmode(mode, uarg);
}

static int16_t rol(enum e_addressMode mode, union u16 uarg) {
  uint8_t mem, oldbit = 0;
  uint32_t addr = 0;
  int16_t result = 0;

  if (mode == ACCUMULATOR) {
    oldbit = !!(__cpu_reg.a & 0x80);
    __cpu_reg.a <<= 1;
    __cpu_reg.a |= __cpu_reg.p.C;
    __cpu_reg.p.C = oldbit;
    result = __cpu_reg.a;
  } else {
    addr = addressmode(mode, uarg);
    mem = readbus(addr);
    oldbit = !!(mem & 0x80);
    mem <<= 1;
    mem |= __cpu_reg.p.C;
    writebus(addr, mem);
    __cpu_reg.p.C = oldbit;
    result = mem;
  }

  return result;
}

static int16_t ror(enum e_addressMode mode, union u16 uarg) {
  uint8_t mem, oldbit = 0;
  uint32_t addr = 0;
  int16_t result = 0;

  if (mode == ACCUMULATOR) {
    oldbit = __cpu_reg.a & 0x01;
    __cpu_reg.a >>= 1;
    __cpu_reg.a |= (__cpu_reg.p.C << 7);
    __cpu_reg.p.C = oldbit;
    result = __cpu_reg.a;
  } else {
    addr = addressmode(mode, uarg);
    mem = readbus(addr);
    oldbit = mem & 0x01;
    mem >>= 1;
    mem |= (__cpu_reg.p.C << 7);
    writebus(addr, mem);
    __cpu_reg.p.C = oldbit;
    result = mem;
  }

  return result;
}

static int16_t pha(enum e_addressMode mode, union u16 uarg) {
  writebus(0x01ff - __cpu_reg.sp--, __cpu_reg.a);
  return 0;
}

static int16_t php(enum e_addressMode mode, union u16 uarg) {
  writebus(0x01ff - __cpu_reg.sp--, __cpu_reg.p.value);
  return 0;
}

static int16_t pla(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.a = readbus(0x01ff - ++__cpu_reg.sp);
}

static int16_t plp(enum e_addressMode mode, union u16 uarg) {
  return __cpu_reg.p.value = readbus(0x01ff - ++__cpu_reg.sp);
}

static int16_t jmp_absolute(enum e_addressMode mode, union u16 uarg) {
  __cpu_reg.pc = uarg.value;
  return 0;
}

static int16_t jsr(enum e_addressMode mode, union u16 uarg) {
  union u16 pc = { .value = __cpu_reg.pc };
  pc.value += 3 - 1; // jsr size equal 3 minus one size of the rts opcode

  debug("%x %x %x", pc.value, pc.msb, pc.lsb);

  writebus(0x01ff - __cpu_reg.sp, pc.msb);
  __cpu_reg.sp--;
  writebus(0x01ff - __cpu_reg.sp, pc.lsb);
  __cpu_reg.sp--;
  __cpu_reg.pc = uarg.value;
  return 0;
}

static int16_t rts(enum e_addressMode mode, union u16 uarg) {
  union u16 pc = { 0 };

  pc.lsb = readbus(0x01ff - ++__cpu_reg.sp);
  pc.msb = readbus(0x01ff - ++__cpu_reg.sp);
  __cpu_reg.pc = pc.value;
  return 0;
}

struct instruction tab[] = {
  {"ADC",    0x69,    IMMEDIATE,    2,   2,   0,   &adc,               &sp_zn},
  {"ADC",    0x65,    ZEROPAGE,    2,   3,   0,   &adc,             &sp_zn},
  {"ADC",    0x75,    ZEROPAGEX,    2,   4,   0,   &adc,             &sp_zn},
  {"ADC",    0x6d,    ABSOLUTE,    3,   4,   0,   &adc,             &sp_zn},
  {"ADC",    0x7d,    ABSOLUTEX,    3,   4,   1,   &adc,             &sp_zn},
  {"ADC",    0x79,    ABSOLUTEY,    3,   4,   0,   &adc,             &sp_zn},
  {"ADC",    0x61,    INDIRECTX,    2,   6,   0,   &adc,             &sp_zn},
  {"ADC",    0x71,    INDIRECTY,    2,   5,   1,   &adc,             &sp_zn},

  {"AND",    0x29,    IMMEDIATE,   2,   2,  0,&and,              &sp_zn},
  {"AND",    0x25,    ZEROPAGE,   2,   3,  0,&and,              &sp_zn},
  {"AND",    0x35,    ZEROPAGEX,   2,   4,  0,&and,              &sp_zn},
  {"AND",    0x2d,    ABSOLUTE,   3,   4,  0,&and,              &sp_zn},
  {"AND",    0x3d,    ABSOLUTEX,   3,   4,  1,&and,              &sp_zn},
  {"AND",    0x39,    ABSOLUTEY,   3,   4,  0,&and,              &sp_zn},
  {"AND",    0x21,    INDIRECTX,   2,   6,  0,&and,              &sp_zn},
  {"AND",    0x31,    INDIRECTY,   2,   6,  1,&and,              &sp_zn},

  {"ASL",    0x0a,    ACCUMULATOR,1,  2,  0,  &asl,              &sp_zn},
  {"ASL",    0x06,    ZEROPAGE,    2,  5,  0,  &asl,              &sp_zn},
  {"ASL",    0x16,    ZEROPAGEX,  2,  6,  0,  &asl,              &sp_zn},
  {"ASL",    0x0e,    ABSOLUTE,    3,  6,  0,  &asl,              &sp_zn},
  {"ASL",    0x1e,    ABSOLUTEX,  3,  7,  0,  &asl,              &sp_zn},

  {"LSR",    0x4a,    ACCUMULATOR,1,  2,  0,  &lsr,              &sp_zn},
  {"LSR",    0x46,    ZEROPAGE,    2,  5,  0,  &lsr,              &sp_zn},
  {"LSR",    0x56,    ZEROPAGEX,  2,  6,  0,  &lsr,              &sp_zn},
  {"LSR",    0x4e,    ABSOLUTE,    3,  6,  0,  &lsr,              &sp_zn},
  {"LSR",    0x5e,    ABSOLUTEX,  3,  7,  0,  &lsr,              &sp_zn},

  {"BCC",    0x90,    RELATIVE,    2,  2,  1,  &bcc,              NULL},
  {"BCS",    0xb0,    RELATIVE,    2,  2,  1,  &bcs,              NULL},
  {"BEQ",    0xf0,    RELATIVE,    2,  2,  1,  &beq,              NULL},
  {"BMI",    0x30,    RELATIVE,    2,  2,  1,  &bmi,              NULL},
  {"BNE",    0xd0,    RELATIVE,    2,  2,  1,  &bne,              NULL},
  {"BPL",    0x10,    RELATIVE,    2,  2,  1,  &bpl,              NULL},
  {"BVC",    0x50,    RELATIVE,    2,  2,  1,  &bvc,              NULL},
  {"BVS",    0x70,    RELATIVE,    2,  2,  1,  &bvs,              NULL},

  {"BIT",    0x24,    ZEROPAGE,    2,  3,  0,  &bit,              NULL},
  {"BIT",    0x2c,    ABSOLUTE,    3,  4,  0,  &bit,              NULL},

  {"CLC",    0x18,    IMPLIED,    1,  2,  0,  &clc,              NULL},
  {"CLD",    0xd8,    IMPLIED,    1,  2,  0,  &cld,              NULL},
  {"CLI",    0x58,    IMPLIED,    1,  2,  0,  &cli,              NULL},
  {"CLD",    0xb8,    IMPLIED,    1,  2,  0,  &clv,              NULL},

  {"SBC",    0xe9,    IMMEDIATE,  2,  2,  0,  &sbc,              &sp_zn},
  {"SBC",    0xe5,    ZEROPAGE,    2,  3,  0,  &sbc,              &sp_zn},
  {"SBC",    0xf5,    ZEROPAGEX,  2,  4,  0,  &sbc,              &sp_zn},
  {"SBC",    0xed,    ABSOLUTE,    3,  4,  0,  &sbc,              &sp_zn},
  {"SBC",    0xfd,    ABSOLUTEX,  3,  4,  1,  &sbc,              &sp_zn},
  {"SBC",    0xf9,    ABSOLUTEY,  3,  4,  0,  &sbc,              &sp_zn},
  {"SBC",    0xe1,    INDIRECTX,  2,  6,  0,  &sbc,              &sp_zn},
  {"SBC",    0xf1,    INDIRECTY,  2,  6,  1,  &sbc,              &sp_zn},

  {"SEC",    0x38,    IMPLIED,    1,  2,  0,  &sec,              NULL},
  {"SED",    0xf8,    IMPLIED,    1,  2,  0,  &sed,              NULL},
  {"SEI",    0x78,    IMPLIED,    1,  2,  0,  &sei,              NULL},

  {"TAX",    0xaa,    IMPLIED,    1,  2,  0,  &tax,              &sp_zn},
  {"TAY",    0xa8,    IMPLIED,    1,  2,  0,  &tay,              &sp_zn},
  {"TSX",    0xba,    IMPLIED,    1,  2,  0,  &tsx,              &sp_zn},
  {"TXA",    0x8a,    IMPLIED,    1,  2,  0,  &txa,              &sp_zn},
  {"TXS",    0x9a,    IMPLIED,    1,  2,  0,  &txs,              NULL},
  {"TYA",    0x98,    IMPLIED,    1,  2,  0,  &tya,              &sp_zn},

  {"CMP",    0xc9,    IMMEDIATE,  2,  2,  0,  &cmp,              &sp_zn},
  {"CMP",    0xc5,    ZEROPAGE,    2,  3,  0,  &cmp,              &sp_zn},
  {"CMP",    0xd5,    ZEROPAGEX,  2,  4,  0,  &cmp,              &sp_zn},
  {"CMP",    0xcd,    ABSOLUTE,    3,  4,  0,  &cmp,              &sp_zn},
  {"CMP",    0xdd,    ABSOLUTEX,  3,  4,  1,  &cmp,              &sp_zn},
  {"CMP",    0xd9,    ABSOLUTEY,  3,  4,  0,  &cmp,              &sp_zn},
  {"CMP",    0xc1,    INDIRECTX,  2,  6,  0,  &cmp,              &sp_zn},
  {"CMP",    0xd1,    INDIRECTY,  2,  5,  1,  &cmp,              &sp_zn},

  {"CPX",    0xe1,    IMMEDIATE,  2,  2,  0,  &cpx,              &sp_zn},
  {"CPX",    0xe4,    ZEROPAGE,    2,  3,  0,  &cpx,              &sp_zn},
  {"CPX",    0xec,    ABSOLUTE,    2,  4,  0,  &cpx,              &sp_zn},

  {"CPY",    0xc0,    IMMEDIATE,  2,  2,  0,  &cpy,              &sp_zn},
  {"CPY",    0xc4,    ZEROPAGE,    2,  3,  0,  &cpy,              &sp_zn},
  {"CPY",    0xcc,    ABSOLUTE,    2,  4,  0,  &cpy,              &sp_zn},

  {"DEC",    0xc6,    ZEROPAGE,    2,  5,  0,  &dec,              &sp_zn},
  {"DEC",    0xd6,    ZEROPAGEX,  2,  6,  0,  &dec,              &sp_zn},
  {"DEC",    0xce,    ABSOLUTE,    3,  6,  0,  &dec,              &sp_zn},
  {"DEC",    0xde,    ABSOLUTEX,  3,  7,  0,  &dec,              &sp_zn},

  {"INC",    0xe6,    ZEROPAGE,    2,  5,  0,  &inc,              &sp_zn},
  {"INC",    0xd6,    ZEROPAGEX,  2,  6,  0,  &inc,              &sp_zn},
  {"INC",    0xee,    ABSOLUTE,    3,  6,  0,  &inc,              &sp_zn},
  {"INC",    0xfe,    ABSOLUTEX,  3,  7,  0,  &inc,              &sp_zn},

  {"DEX",    0xca,    IMPLIED,    1,  2,  0,  &dex,              &sp_zn},
  {"DEY",    0x88,    IMPLIED,    1,  2,  0,  &dey,              &sp_zn},
  {"INX",    0xe8,    IMPLIED,    1,  2,  0,  &inx,              &sp_zn},
  {"INY",    0xc8,    IMPLIED,    1,  2,  0,  &iny,              &sp_zn},

  {"EOR",    0x49,    IMMEDIATE,  2,  2,  0,  &eor,              &sp_zn},
  {"EOR",    0x45,    ZEROPAGE,    2,  3,  0,  &eor,              &sp_zn},
  {"EOR",    0x55,    ZEROPAGEX,  2,  4,  0,  &eor,              &sp_zn},
  {"EOR",    0x4d,    ABSOLUTE,    3,  4,  0,  &eor,              &sp_zn},
  {"EOR",    0x5d,    ABSOLUTEX,  3,  4,  1,  &eor,              &sp_zn},
  {"EOR",    0x59,    ABSOLUTEY,  3,  4,  0,  &eor,              &sp_zn},
  {"EOR",    0x41,    INDIRECTX,  2,  6,  0,  &eor,              &sp_zn},
  {"EOR",    0x51,    INDIRECTY,  2,  6,  1,  &eor,              &sp_zn},

  {"LDA",    0xa9,    IMMEDIATE,  2,  2,  0,  &lda,              &sp_zn},
  {"LDA",    0xa5,    ZEROPAGE,    2,  3,  0,  &lda,              &sp_zn},
  {"LDA",    0xb5,    ZEROPAGEX,  2,  4,  0,  &lda,              &sp_zn},
  {"LDA",    0xad,    ABSOLUTE,    3,  4,  0,  &lda,              &sp_zn},
  {"LDA",    0xbd,    ABSOLUTEX,  3,  4,  1,  &lda,              &sp_zn},
  {"LDA",    0xb9,    ABSOLUTEY,  3,  4,  0,  &lda,              &sp_zn},
  {"LDA",    0xa1,    INDIRECTX,  2,  6,  0,  &lda,              &sp_zn},
  {"LDA",    0xb1,    INDIRECTY,  2,  6,  1,  &lda,              &sp_zn},

  {"LDX",    0xa2,    IMMEDIATE,  2,  2,  0,  &ldx,              &sp_zn},
  {"LDX",    0xa6,    ZEROPAGE,    2,  3,  0,  &ldx,              &sp_zn},
  {"LDX",    0xb6,    ZEROPAGEY,  2,  4,  0,  &ldx,              &sp_zn},
  {"LDX",    0xae,    ABSOLUTE,    3,  4,  0,  &ldx,              &sp_zn},
  {"LDX",    0xbe,    ABSOLUTEY,  3,  4,  1,  &ldx,              &sp_zn},

  {"LDY",    0xa0,    IMMEDIATE,  2,  2,  0,  &ldy,              &sp_zn},
  {"LDY",    0xa4,    ZEROPAGE,    2,  3,  0,  &ldy,              &sp_zn},
  {"LDY",    0xb4,    ZEROPAGEX,  2,  4,  0,  &ldy,              &sp_zn},
  {"LDY",    0xac,    ABSOLUTE,    3,  4,  0,  &ldy,              &sp_zn},
  {"LDY",    0xbc,    ABSOLUTEX,  3,  4,  1,  &ldy,              &sp_zn},

  {"STA",    0x85,    ZEROPAGE,    2,  3,  0,  &sta,              NULL},
  {"STA",    0x95,    ZEROPAGEX,  2,  4,  0,  &sta,              NULL},
  {"STA",    0x8d,    ABSOLUTE,    3,  4,  0,  &sta,              NULL},
  {"STA",    0x9d,    ABSOLUTEX,  3,  4,  1,  &sta,              NULL},
  {"STA",    0x99,    ABSOLUTEY,  3,  4,  0,  &sta,              NULL},
  {"STA",    0x81,    INDIRECTX,  2,  6,  0,  &sta,              NULL},
  {"STA",    0x91,    INDIRECTY,  2,  6,  1,  &sta,              NULL},

  {"STX",    0x86,    ZEROPAGE,    2,  3,  0,  &stx,              NULL},
  {"STX",    0x96,    ZEROPAGEX,  2,  4,  0,  &stx,              NULL},
  {"STX",    0x8e,    ABSOLUTE,    3,  4,  0,  &stx,              NULL},

  {"STY",    0x84,    ZEROPAGE,    2,  3,  0,  &sty,              NULL},
  {"STY",    0x94,    ZEROPAGEX,  2,  4,  0,  &sty,              NULL},
  {"STY",    0x8c,    ABSOLUTE,    3,  4,  0,  &sty,              NULL},

  {"ORA",    0x09,    IMMEDIATE,   2,   2,  0,  &ora,              &sp_zn},
  {"ORA",    0x05,    ZEROPAGE,   2,   3,  0,  &ora,              &sp_zn},
  {"ORA",    0x15,    ZEROPAGEX,   2,   4,  0,  &ora,              &sp_zn},
  {"ORA",    0x0d,    ABSOLUTE,   3,   4,  0,  &ora,              &sp_zn},
  {"ORA",    0x1d,    ABSOLUTEX,   3,   4,  1,  &ora,              &sp_zn},
  {"ORA",    0x19,    ABSOLUTEY,   3,   4,  0,  &ora,              &sp_zn},
  {"ORA",    0x01,    INDIRECTX,   2,   6,  0,  &ora,              &sp_zn},
  {"ORA",    0x11,    INDIRECTY,   2,   6,  1,  &ora,              &sp_zn},

  {"ROR",    0x6a,    ACCUMULATOR,1,  2,  0,  &ror,              &sp_zn},
  {"ROR",    0x66,    ZEROPAGE,    2,  5,  0,  &ror,              &sp_zn},
  {"ROR",    0x76,    ZEROPAGEX,  2,  6,  0,  &ror,              &sp_zn},
  {"ROR",    0x6e,    ABSOLUTE,    3,  6,  0,  &ror,              &sp_zn},
  {"ROR",    0x7e,    ABSOLUTEX,  3,  7,  0,  &ror,              &sp_zn},

  {"ROL",    0x2a,    ACCUMULATOR,1,  2,  0,  &rol,              &sp_zn},
  {"ROL",    0x26,    ZEROPAGE,    2,  5,  0,  &rol,              &sp_zn},
  {"ROL",    0x36,    ZEROPAGEX,  2,  6,  0,  &rol,              &sp_zn},
  {"ROL",    0x2e,    ABSOLUTE,    3,  6,  0,  &rol,              &sp_zn},
  {"ROL",    0x3e,    ABSOLUTEX,  3,  7,  0,  &rol,              &sp_zn},

  {"PHA",    0x48,    IMPLIED,    1,  3,  0,  &pha,              NULL},
  {"PHP",    0x08,    IMPLIED,    1,  3,  0,  &php,              NULL},
  {"PLA",    0x68,    IMPLIED,    1,  3,  0,  &pla,              &sp_zn},
  {"PLP",    0x28,    IMPLIED,    1,  3,  0,  &plp,              NULL},

  {"JMP",    0x4c,    ABSOLUTE,    3,  3,  0,  &jmp_absolute,    NULL},
  {"JMP",    0x6c,    INDIRECT,    3,  5,  0,  &not_found,        NULL}, // TODO

  {"JSR",    0x20,    ABSOLUTE,    3,  6,  0,  &jsr,              NULL},
  {"RTS",    0x60,    IMPLIED,    1,  6,  0,  &rts,              NULL},

  {"NOP",    0xea,    IMPLIED,    1,  2,  0,  &nop,              NULL},
  {"BRK",    0x00,    IMPLIED,    1,  7,  0,  &brk,              NULL},

};

/**
 *
 * return boolean when instruction completed
 */
static int handle_op(struct instruction op) {

  static int cycles_remaining = 0;
  static union u16 uarg = (union u16){ .value = 0 };

  if (cycles_remaining > 1) {
    cycles_remaining--;
    return 0;
  } else if (cycles_remaining == 0) {
    // 1. read the arg in function of addressMode

    uarg = read_arg(op.mode);

    // 2. page crossed
    if (op.crossed) {
      if (uarg.lsb + __cpu_reg.x > 0xff) {
        op.cycles++;
      }
    }

    // 3. cycles
    cycles_remaining = op.cycles - 1;
    return 0;
  }
  cycles_remaining = 0;

  // 4. debug
  debug_start();
  debug_opcode(op.mode, op.str, op.code, uarg);


  uint16_t pc = __cpu_reg.pc;
  // 6. algo

  __no_rw_debug = 0; // enable debug r/w
  uint16_t result = op.fn(op.mode, uarg);

  // 5. increment pc
  if (strcmp(op.str, "JMP") != 0 && strcmp(op.str, "JSR") != 0) // jmp and jsr jump to a specific location
    __cpu_reg.pc += op.size;

  // 7. set to registers Z & N
  if (op.end)
    op.end(result);


  debug_end();
  return 1;

}

void cpu_init_op_tab() {
  for (int i = 0; i < 0x100; i++) {
    _op[i] = (struct instruction){ "UNK", i, IMPLIED, 1, 1, 0, &not_found, NULL };
  }
  for (int i = 0; i < (sizeof(tab) / sizeof(struct instruction)); i++) {
    _op[tab[i].code] = tab[i];
  }
}

// https://www.pagetable.com/?p=410
void cpu_init()
{

  if (!cpu_readbus16) {
    readbus16 = &readbus16_default;
  } else {
    readbus16 = cpu_readbus16;
  }

  // assign read/write bus to local variable and assert it
  // failed if not assigned before init
  assert(cpu_readbus);
  readbus = cpu_readbus;
  assert(cpu_writebus);
  writebus = cpu_writebus;

  cpu_init_op_tab();

  // https://www.nesdev.org/wiki/CPU_power_up_state
  __cpu_reg.pc = readbus16(0xfffc).value;
  __cpu_reg.a = __cpu_reg.x = __cpu_reg.y = 0;
  __cpu_reg.p.value = 0x34;
  __cpu_reg.sp = 0xfd;
  writebus(0x4017, 0); // Warning writebus whereas the cpu is not started yet ! TODO: check the behaviour
  writebus(0x4015, 0);
  for (int i = 0; i <= 0xf; i++)
    writebus(0x4000 + i, 16);
  for (int i = 0; i <= 0x3; i++)
    writebus(0x4010 + i, 16);

  char* env_brk = getenv("BRK");
  char* env_assert_pc = getenv("ASSERT_PC");
  __debug_brk = env_brk ? (strcmp(env_brk, "") == 0 ? 0 : strtol(env_brk, NULL, 16)) : 0;
  __debug_assert_pc = env_assert_pc ? strcmp(env_assert_pc, "1") == 0 : 0;
}

void cpu_irq()
{
  union u16 pc = { .value = __cpu_reg.pc };
  pc.value = __cpu_reg.pc += 2;
  writebus(0x01ff - __cpu_reg.sp--, pc.msb);
  writebus(0x01ff - __cpu_reg.sp--, pc.lsb);
  writebus(0x01ff - __cpu_reg.sp--, __cpu_reg.p.value);
  union u16 brk = readbus16(0xfffe);
  __cpu_reg.pc = brk.value;
  __cpu_reg.p.B = 0; // clear b flags
}

// global function
void reset()
{
  // https://www.pagetable.com/?p=410
  // TODO
}

static int cpu_pc_assert(enum e_cpu_code code) {

  if (__cpu_reg.pc == __debug_brk) {
    return __debug_assert_pc ? CPU_DEBUG_PC_ASSERT : CPU_DEBUG_PC_BREAK;
  }
  return code;
}

enum e_cpu_code cpu_exec()
{

  static enum e_cpu_code cpu_code = CPU_INSTRUCTION_COMPLETE;
  static uint8_t op = 0;

  // disable readbus on opcode parsing
  __no_rw_debug = 1;

  // 1. read op
  if (cpu_code == CPU_INSTRUCTION_COMPLETE)
    op = readbus(__cpu_reg.pc);

#ifdef DEBUG_CPU
  if (op == 0)
  {
    // break;
    debug("DEBUG_CPU BREAK");
    cpu_code = CPU_INSTRUCTION_COMPLETE;
    return CPU_BREAK;
  }
#endif

  // 2. handle op pipeline
  cpu_code = handle_op(_op[op]);

  // 3. handle debugging
  cpu_code = cpu_pc_assert(cpu_code);

  debug("STATUS %d", cpu_code);

  return cpu_code;
}
