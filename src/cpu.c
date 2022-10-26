#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cpu.h"
#include "debug.h"
#include "log.h"

#define debug(...) log_x(LOG_CPU, __VA_ARGS__)

// extern global var
t_mem __cpu_memory[MEM_SIZE] = {0};

readwritefn __bus_read_on_array[CPU_EVENT_BUS_FN_SIZE] = {NULL};
size_t __bus_read_on_size = 0;
readwritefn __bus_write_on_array[CPU_EVENT_BUS_FN_SIZE] = {NULL};
size_t __bus_write_on_size = 0;

struct instruction _op[0xff] = {0};

t_registers __cpu_reg = {
	.pc = 0,
	.sp = 0,
	.p = 0,
	.a = 0,
	.x = 0,
	.y = 0};

readwritefn cpu_read_on(readwritefn fn) {
  if (!fn) return NULL;
  for (int i = 0; i < __bus_read_on_size; i++) {
    if (fn == __bus_read_on_array[i]) return NULL;
  }
  __bus_read_on_array[__bus_read_on_size] = fn;
  __bus_read_on_size++;

  return fn;
}

readwritefn cpu_write_on(readwritefn fn) {
  if (!fn) return NULL;
  for (int i = 0; i < __bus_write_on_size; i++) {
    if (fn == __bus_write_on_array[i]) return NULL;
  }
  __bus_write_on_array[__bus_write_on_size] = fn;
  __bus_write_on_size++;

  return fn;
}

static uint8_t readbus(uint32_t addr) {

  debug("READ=0x%x VALUE=%d/%d/0x%x", addr, *__cpu_memory[addr], (int8_t)*__cpu_memory[addr], *__cpu_memory[addr]);

  uint8_t value = *__cpu_memory[addr];
  for (int i = 0; i < __bus_read_on_size; i++) {
    value = __bus_read_on_array[i](value, addr);
  }

  return value;
}
uint8_t cpu_readbus(uint32_t addr) {
	return readbus(addr);
}

static union u16 readbus16(uint32_t addr) {

  debug("READ16=0x%x", addr);

  union u16 v = {.lsb = readbus(addr), .msb = readbus(addr + 1)};
  return v;
}
union u16 cpu_readbus16(uint32_t addr) {
	return readbus16(addr);
}

static uint8_t readbus_pc() {
  return readbus(__cpu_reg.pc + 1);
}

static union u16 readbus16_pc() {
  return readbus16(__cpu_reg.pc + 1);
}

static void writebus(uint32_t addr, uint8_t value) {

  debug("WRITE=0x%x VALUE=%d/%d", addr, value, (int8_t)value);

  for (int i = 0; i < __bus_write_on_size; i++) {
    value = __bus_write_on_array[i](value, addr);
  }

  *__cpu_memory[addr] = value;
}
void cpu_writebus(uint32_t addr, uint8_t value) {
	return writebus(addr, value);
}

static void debug_opcode(enum e_addressMode mode, char *str, uint8_t op, union u16 arg) {

	switch (mode)
	{
	case ACCUMULATOR:
		debug("$%04x    %02x           %s", __cpu_reg.pc, op, str);
		break;
	case IMPLIED:
		debug("$%04x    %02x           %s", __cpu_reg.pc, op, str);
		break;
	case IMMEDIATE:
		debug("$%04x    %02x %02x        %s #$%02x", __cpu_reg.pc, op, arg, str, arg.lsb);
		break;
	case ABSOLUTE:
		debug("$%04x    %02x %02x %02x     %s $%04x", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
		break;
	case ZEROPAGE:
		debug("$%04x    %02x %02x        %s $%02x", __cpu_reg.pc, op, arg, str, arg.lsb);
		break;
	case RELATIVE:
		debug("$%04x    %02x %02x        %s $%02x", __cpu_reg.pc, op, arg, str, arg.lsb);
		break;
	case ABSOLUTEX:
		debug("$%04x    %02x %02x %02x     %s $%04x,X", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
		break;
	case ABSOLUTEY:
		debug("$%04x    %02x %02x %02x     %s $%04x,Y", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
		break;
	case ZEROPAGEX:
		debug("$%04x    %02x %02x        %s $%02x,X", __cpu_reg.pc, op, arg, str, arg.lsb);
		break;
	case ZEROPAGEY:
		debug("$%04x    %02x %02x        %s $%02x,Y", __cpu_reg.pc, op, arg, str, arg.lsb);
		break;
	case INDIRECT:
		debug("$%04x    %02x %02x %02x     %s ($%04x)", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
		break;
	case INDIRECTX:
		debug("$%04x    %02x %02x        %s ($%02x,X)", __cpu_reg.pc, op, arg, str, arg.lsb);
		break;
	case INDIRECTY:
		debug("$%04x    %02x %02x        %s ($%02x,Y)", __cpu_reg.pc, op, arg, str, arg.lsb);
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
		return (union u16){.value = 0};
	case IMPLIED:
		return (union u16){.value = 0};
	case IMMEDIATE:
		return (union u16){.value = readbus_pc()};
	case ABSOLUTE:
		return readbus16_pc();
	case ZEROPAGE:
		return (union u16){.value = readbus_pc()};
	case RELATIVE:
		return (union u16){.value = readbus_pc()};
	case ABSOLUTEX:
		return readbus16_pc();
	case ABSOLUTEY:
		return readbus16_pc();
	case ZEROPAGEX:
		return (union u16){.value = readbus_pc()};
	case ZEROPAGEY:
		return (union u16){.value = readbus_pc()};
	case INDIRECT:
		return readbus16_pc();
	case INDIRECTX:
		return (union u16){.value = readbus_pc()};
	case INDIRECTY:
		return (union u16){.value = readbus_pc()};
	}

	return (union u16){.value = 0};
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
	int16_t result = __cpu_reg.a += readDataFromAddressmode(mode, uarg) + __cpu_reg.p.C;

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

	return result;
}

static int16_t and(enum e_addressMode mode, union u16 uarg) {
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

	int8_t value = __cpu_reg.a & readbus(addressmode(mode, uarg));

	__cpu_reg.p.Z = !value;
	__cpu_reg.p.V = !!(value & 0x40);
	__cpu_reg.p.N = !!(value & 0x80);

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
	return __cpu_reg.a = readbus(0x01ff - __cpu_reg.sp++);
}

static int16_t plp(enum e_addressMode mode, union u16 uarg) {
	return __cpu_reg.p.value = readbus(0x01ff - __cpu_reg.sp++);
}

static int16_t jmp_absolute(enum e_addressMode mode, union u16 uarg) {
	__cpu_reg.pc = uarg.value;
	return 0;
}

static int16_t jsr(enum e_addressMode mode, union u16 uarg) {
	union u16 pc = {.value = __cpu_reg.pc};

	writebus(0x01ff - __cpu_reg.sp, pc.msb);
	__cpu_reg.sp--;
	writebus(0x01ff - __cpu_reg.sp, pc.lsb);
	__cpu_reg.sp--;
	__cpu_reg.pc = uarg.value;
	return 0;
}

static int16_t rts(enum e_addressMode mode, union u16 uarg) {
	union u16 pc = {0};

	pc.lsb = readbus(0x01ff - ++__cpu_reg.sp);
	pc.msb = readbus(0x01ff - ++__cpu_reg.sp);
	__cpu_reg.pc = pc.value;
	return 0;
}

struct instruction tab[] = {
	{"ADC", 	0x69, 	IMMEDIATE, 	2, 	2, 	0, 	&adc,						 	&sp_zn},
	{"ADC", 	0x65, 	ZEROPAGE, 	2, 	3, 	0, 	&adc, 						&sp_zn},
	{"ADC", 	0x75, 	ZEROPAGEX, 	2, 	4, 	0, 	&adc, 						&sp_zn},
	{"ADC", 	0x6d, 	ABSOLUTE, 	3, 	4, 	0, 	&adc, 						&sp_zn},
	{"ADC", 	0x7d, 	ABSOLUTEX, 	3, 	4, 	1, 	&adc, 						&sp_zn},
	{"ADC", 	0x79, 	ABSOLUTEY, 	3, 	4, 	0, 	&adc, 						&sp_zn},
	{"ADC", 	0x61, 	INDIRECTX, 	2, 	6, 	0, 	&adc, 						&sp_zn},
	{"ADC", 	0x71, 	INDIRECTY, 	2, 	5, 	1, 	&adc, 						&sp_zn},

	{"AND",		0x29,		IMMEDIATE, 	2, 	2,	0,	&and,							&sp_zn},
	{"AND",		0x25,		ZEROPAGE, 	2, 	3,	0,	&and,							&sp_zn},
	{"AND",		0x35,		ZEROPAGEX, 	2, 	4,	0,	&and,							&sp_zn},
	{"AND",		0x2d,		ABSOLUTE, 	3, 	4,	0,	&and,							&sp_zn},
	{"AND",		0x3d,		ABSOLUTEX, 	3, 	4,	1,	&and,							&sp_zn},
	{"AND",		0x39,		ABSOLUTEY, 	3, 	4,	0,	&and,							&sp_zn},
	{"AND",		0x21,		INDIRECTX, 	2, 	6,	0,	&and,							&sp_zn},
	{"AND",		0x31,		INDIRECTY, 	2, 	6,	1,	&and,							&sp_zn},

	{"ASL",		0x0a,		ACCUMULATOR,1,	2,	0,	&asl,							&sp_zn},
	{"ASL",		0x06,		ZEROPAGE,		2,	5,	0,	&asl,							&sp_zn},
	{"ASL",		0x16,		ZEROPAGEX,	2,	6,	0,	&asl,							&sp_zn},
	{"ASL",		0x0e,		ABSOLUTE,		3,	6,	0,	&asl,							&sp_zn},
	{"ASL",		0x0e,		ABSOLUTEX,	3,	7,	0,	&asl,							&sp_zn},
	
	{"LSR",		0x4a,		ACCUMULATOR,1,	2,	0,	&lsr,							&sp_zn},
	{"LSR",		0x46,		ZEROPAGE,		2,	5,	0,	&lsr,							&sp_zn},
	{"LSR",		0x56,		ZEROPAGEX,	2,	6,	0,	&lsr,							&sp_zn},
	{"LSR",		0x4e,		ABSOLUTE,		3,	6,	0,	&lsr,							&sp_zn},
	{"LSR",		0x5e,		ABSOLUTEX,	3,	7,	0,	&lsr,							&sp_zn},

	{"BCC",		0x90,		RELATIVE,		2,	2,	1,	&bcc,							NULL},
	{"BCS",		0xb0,		RELATIVE,		2,	2,	1,	&bcs,							NULL},
	{"BEQ",		0xf0,		RELATIVE,		2,	2,	1,	&beq,							NULL},
	{"BMI",		0x30,		RELATIVE,		2,	2,	1,	&bmi,							NULL},
	{"BNE",		0xd0,		RELATIVE,		2,	2,	1,	&bne,							NULL},
	{"BPL",		0x10,		RELATIVE,		2,	2,	1,	&bpl,							NULL},
	{"BVC",		0x50,		RELATIVE,		2,	2,	1,	&bvc,							NULL},
	{"BVS",		0x70,		RELATIVE,		2,	2,	1,	&bvs,							NULL},

	{"BIT",		0x24,		ZEROPAGE,		2,	3,	0,	&bit,							NULL},
	{"BIT",		0x2c,		ABSOLUTE,		3,	4,	0,	&bit,							NULL},

	{"CLC",		0x18,		IMPLIED,		1,	2,	0,	&clc,							NULL},
	{"CLD",		0xd8,		IMPLIED,		1,	2,	0,	&cld,							NULL},
	{"CLI",		0x58,		IMPLIED,		1,	2,	0,	&cli,							NULL},
	{"CLD",		0xb8,		IMPLIED,		1,	2,	0,	&clv,							NULL},

	{"SBC",		0xe9,		IMMEDIATE,	2,	2,	0,	&sbc,							&sp_zn},
	{"SBC",		0xe5,		ZEROPAGE,		2,	3,	0,	&sbc,							&sp_zn},
	{"SBC",		0xf5,		ZEROPAGEX,	2,	4,	0,	&sbc,							&sp_zn},
	{"SBC",		0xed,		ABSOLUTE,		3,	4,	0,	&sbc,							&sp_zn},
	{"SBC",		0xfd,		ABSOLUTEX,	3,	4,	1,	&sbc,							&sp_zn},
	{"SBC",		0xf9,		ABSOLUTEY,	3,	4,	0,	&sbc,							&sp_zn},
	{"SBC",		0xe1,		INDIRECTX,	2,	6,	0,	&sbc,							&sp_zn},
	{"SBC",		0xf1,		INDIRECTY,	2,	6,	1,	&sbc,							&sp_zn},

	{"SEC",		0x38,		IMPLIED,		1,	2,	0,	&sec,							NULL},
	{"SED",		0xf8,		IMPLIED,		1,	2,	0,	&sed,							NULL},
	{"SEI",		0x78,		IMPLIED,		1,	2,	0,	&sei,							NULL},

	{"TAX",		0xaa,		IMPLIED,		1,	2,	0,	&tax,							&sp_zn},
	{"TAY",		0xa8,		IMPLIED,		1,	2,	0,	&tay,							&sp_zn},
	{"TSX",		0xba,		IMPLIED,		1,	2,	0,	&tsx,							&sp_zn},
	{"TXA",		0x8a,		IMPLIED,		1,	2,	0,	&txa,							&sp_zn},
	{"TXS",		0x9a,		IMPLIED,		1,	2,	0,	&txs,							NULL},
	{"TYA",		0x98,		IMPLIED,		1,	2,	0,	&tya,							&sp_zn},

	{"CMP",		0xc9,		IMMEDIATE,	2,	2,	0,	&cmp,							&sp_zn},
	{"CMP",		0xc5,		ZEROPAGE,		2,	3,	0,	&cmp,							&sp_zn},
	{"CMP",		0xd5,		ZEROPAGEX,	2,	4,	0,	&cmp,							&sp_zn},
	{"CMP",		0xcd,		ABSOLUTE,		3,	4,	0,	&cmp,							&sp_zn},
	{"CMP",		0xdd,		ABSOLUTEX,	3,	4,	1,	&cmp,							&sp_zn},
	{"CMP",		0xd9,		ABSOLUTEY,	3,	4,	0,	&cmp,							&sp_zn},
	{"CMP",		0xc1,		INDIRECTX,	2,	6,	0,	&cmp,							&sp_zn},
	{"CMP",		0xd1,		INDIRECTY,	2,	5,	1,	&cmp,							&sp_zn},

	{"CPX",		0xe1,		IMMEDIATE,	2,	2,	0,	&cpx,							&sp_zn},
	{"CPX",		0xe4,		ZEROPAGE,		2,	3,	0,	&cpx,							&sp_zn},
	{"CPX",		0xec,		ABSOLUTE,		2,	4,	0,	&cpx,							&sp_zn},

	{"CPY",		0xc0,		IMMEDIATE,	2,	2,	0,	&cpy,							&sp_zn},
	{"CPY",		0xc4,		ZEROPAGE,		2,	3,	0,	&cpy,							&sp_zn},
	{"CPY",		0xcc,		ABSOLUTE,		2,	4,	0,	&cpy,							&sp_zn},

	{"DEC",		0xc6,		ZEROPAGE,		2,	5,	0,	&dec,							&sp_zn},
	{"DEC",		0xd6,		ZEROPAGEX,	2,	6,	0,	&dec,							&sp_zn},
	{"DEC",		0xce,		ABSOLUTE,		3,	6,	0,	&dec,							&sp_zn},
	{"DEC",		0xde,		ABSOLUTEX,	3,	7,	0,	&dec,							&sp_zn},
	
	{"INC",		0xe6,		ZEROPAGE,		2,	5,	0,	&inc,							&sp_zn},
	{"INC",		0xd6,		ZEROPAGEX,	2,	6,	0,	&inc,							&sp_zn},
	{"INC",		0xee,		ABSOLUTE,		3,	6,	0,	&inc,							&sp_zn},
	{"INC",		0xfe,		ABSOLUTEX,	3,	7,	0,	&inc,							&sp_zn},

	{"DEX",		0xca,		IMPLIED,		1,	2,	0,	&dex,							&sp_zn},
	{"DEY",		0x88,		IMPLIED,		1,	2,	0,	&dey,							&sp_zn},
	{"INX",		0xe8,		IMPLIED,		1,	2,	0,	&inx,							&sp_zn},
	{"INY",		0xc8,		IMPLIED,		1,	2,	0,	&iny,							&sp_zn},

	{"EOR",		0x49,		IMMEDIATE,	2,	2,	0,	&eor,							&sp_zn},
	{"EOR",		0x45,		ZEROPAGE,		2,	3,	0,	&eor,							&sp_zn},
	{"EOR",		0x55,		ZEROPAGEX,	2,	4,	0,	&eor,							&sp_zn},
	{"EOR",		0x4d,		ABSOLUTE,		3,	4,	0,	&eor,							&sp_zn},
	{"EOR",		0x5d,		ABSOLUTEX,	3,	4,	1,	&eor,							&sp_zn},
	{"EOR",		0x59,		ABSOLUTEY,	3,	4,	0,	&eor,							&sp_zn},
	{"EOR",		0x41,		INDIRECTX,	2,	6,	0,	&eor,							&sp_zn},
	{"EOR",		0x51,		INDIRECTY,	2,	6,	1,	&eor,							&sp_zn},
	
	{"LDA",		0xa9,		IMMEDIATE,	2,	2,	0,	&lda,							&sp_zn},
	{"LDA",		0xa5,		ZEROPAGE,		2,	3,	0,	&lda,							&sp_zn},
	{"LDA",		0xb5,		ZEROPAGEX,	2,	4,	0,	&lda,							&sp_zn},
	{"LDA",		0xad,		ABSOLUTE,		3,	4,	0,	&lda,							&sp_zn},
	{"LDA",		0xbd,		ABSOLUTEX,	3,	4,	1,	&lda,							&sp_zn},
	{"LDA",		0xb9,		ABSOLUTEY,	3,	4,	0,	&lda,							&sp_zn},
	{"LDA",		0xa1,		INDIRECTX,	2,	6,	0,	&lda,							&sp_zn},
	{"LDA",		0xb1,		INDIRECTY,	2,	6,	1,	&lda,							&sp_zn},
	
	{"LDX",		0xa2,		IMMEDIATE,	2,	2,	0,	&ldx,							&sp_zn},
	{"LDX",		0xa6,		ZEROPAGE,		2,	3,	0,	&ldx,							&sp_zn},
	{"LDX",		0xb6,		ZEROPAGEY,	2,	4,	0,	&ldx,							&sp_zn},
	{"LDX",		0xae,		ABSOLUTE,		3,	4,	0,	&ldx,							&sp_zn},
	{"LDX",		0xbe,		ABSOLUTEY,	3,	4,	1,	&ldx,							&sp_zn},
	
	{"LDY",		0xa0,		IMMEDIATE,	2,	2,	0,	&ldy,							&sp_zn},
	{"LDY",		0xa4,		ZEROPAGE,		2,	3,	0,	&ldy,							&sp_zn},
	{"LDY",		0xb4,		ZEROPAGEX,	2,	4,	0,	&ldy,							&sp_zn},
	{"LDY",		0xac,		ABSOLUTE,		3,	4,	0,	&ldy,							&sp_zn},
	{"LDY",		0xbc,		ABSOLUTEX,	3,	4,	1,	&ldy,							&sp_zn},
	
	{"STA",		0x85,		ZEROPAGE,		2,	3,	0,	&sta,							NULL},
	{"STA",		0x95,		ZEROPAGEX,	2,	4,	0,	&sta,							NULL},
	{"STA",		0x8d,		ABSOLUTE,		3,	4,	0,	&sta,							NULL},
	{"STA",		0x9d,		ABSOLUTEX,	3,	4,	1,	&sta,							NULL},
	{"STA",		0x99,		ABSOLUTEY,	3,	4,	0,	&sta,							NULL},
	{"STA",		0x81,		INDIRECTX,	2,	6,	0,	&sta,							NULL},
	{"STA",		0x91,		INDIRECTY,	2,	6,	1,	&sta,							NULL},

	{"STX",		0x86,		ZEROPAGE,		2,	3,	0,	&stx,							NULL},
	{"STX",		0x96,		ZEROPAGEX,	2,	4,	0,	&stx,							NULL},
	{"STX",		0x8e,		ABSOLUTE,		3,	4,	0,	&stx,							NULL},
	
	{"STY",		0x84,		ZEROPAGE,		2,	3,	0,	&sty,							NULL},
	{"STY",		0x94,		ZEROPAGEX,	2,	4,	0,	&sty,							NULL},
	{"STY",		0x8c,		ABSOLUTE,		3,	4,	0,	&sty,							NULL},
	
	{"ORA",		0x09,		IMMEDIATE, 	2, 	2,	0,	&ora,							&sp_zn},
	{"ORA",		0x05,		ZEROPAGE, 	2, 	3,	0,	&ora,							&sp_zn},
	{"ORA",		0x15,		ZEROPAGEX, 	2, 	4,	0,	&ora,							&sp_zn},
	{"ORA",		0x0d,		ABSOLUTE, 	3, 	4,	0,	&ora,							&sp_zn},
	{"ORA",		0x1d,		ABSOLUTEX, 	3, 	4,	1,	&ora,							&sp_zn},
	{"ORA",		0x19,		ABSOLUTEY, 	3, 	4,	0,	&ora,							&sp_zn},
	{"ORA",		0x01,		INDIRECTX, 	2, 	6,	0,	&ora,							&sp_zn},
	{"ORA",		0x11,		INDIRECTY, 	2, 	6,	1,	&ora,							&sp_zn},
	
	{"ROR",		0x6a,		ACCUMULATOR,1,	2,	0,	&ror,							&sp_zn},
	{"ROR",		0x66,		ZEROPAGE,		2,	5,	0,	&ror,							&sp_zn},
	{"ROR",		0x76,		ZEROPAGEX,	2,	6,	0,	&ror,							&sp_zn},
	{"ROR",		0x6e,		ABSOLUTE,		3,	6,	0,	&ror,							&sp_zn},
	{"ROR",		0x7e,		ABSOLUTEX,	3,	7,	0,	&ror,							&sp_zn},
	
	{"ROL",		0x2a,		ACCUMULATOR,1,	2,	0,	&rol,							&sp_zn},
	{"ROL",		0x26,		ZEROPAGE,		2,	5,	0,	&rol,							&sp_zn},
	{"ROL",		0x36,		ZEROPAGEX,	2,	6,	0,	&rol,							&sp_zn},
	{"ROL",		0x2e,		ABSOLUTE,		3,	6,	0,	&rol,							&sp_zn},
	{"ROL",		0x3e,		ABSOLUTEX,	3,	7,	0,	&rol,							&sp_zn},

	{"PHA",		0x48,		IMPLIED,		1,	3,	0,	&pha,							NULL},
	{"PHP",		0x08,		IMPLIED,		1,	3,	0,	&php,							NULL},
	{"PLA",		0x68,		IMPLIED,		1,	3,	0,	&pla,							&sp_zn},
	{"PLP",		0x28,		IMPLIED,		1,	3,	0,	&plp,							NULL},

	{"JMP",		0x4c,		ABSOLUTE,		0,	3,	0,	&jmp_absolute,		NULL},
	{"JMP",		0x6c,		INDIRECT,		3,	5,	0,	&not_found,				NULL}, // TODO

	{"JSR",		0x20,		ABSOLUTE,		3,	6,	0,	&jsr,							NULL},
	{"RTS",		0x60,		IMPLIED,		1,	6,	0,	&rts,							NULL},

	{"NOP",		0xea,		IMPLIED,		1,	2,	0,	&nop,							NULL},
	{"BRK",		0x00,		IMPLIED,		1,	7,	0,	&brk,							NULL},

};

static void handle_op(struct instruction op) {

	static int cycles_remaining = 0;
	static int cycles_done = false;

	// TODO: Handle reset
	if (cycles_remaining) {
		cycles_remaining--;
		cycles_done = !cycles_remaining;
		return ;
	}

	// 1. read the arg in function of addressMode
	union u16 uarg = read_arg(op.mode);

	// 2. page crossed
	if (op.crossed) {
		if (uarg.lsb + __cpu_reg.x > 0xff) {
			op.cycles++;
		}
	}

	// 3. cycles
	if (!cycles_done) {
		cycles_remaining = op.cycles - 1;
		return ;
	}

	// 4. debug
	debug_opcode(op.mode, op.str, op.code, uarg);

	// 5. increment pc
	__cpu_reg.pc += op.size;

	// 6. algo
	uint16_t result = op.fn(op.mode, uarg);

	// 7. set to registers Z & N
	if (op.end)
		op.end(result);

}

// https://www.pagetable.com/?p=410
void cpu_init()
{
	for (int i = 0; i < 0xff; i++) {
		_op[i] = (struct instruction){"UNK", i, IMPLIED, 1, 1, 0, &not_found, NULL};
	}
	for (int i = 0; i < (sizeof(tab)/sizeof(struct instruction)); i++) {
		_op[tab[i].code] = tab[i];
	}

	t_registers *reg = &__cpu_reg;
	t_mem *memory = __cpu_memory; 

	assert(MEM_SIZE == 64 * 1024);
	reg->pc = readbus16(0xfffc).value;
	reg->a = reg->x = reg->y = 0;
	reg->p.value = 0x34;
	reg->sp = 0xfd;
	*memory[0x4017] = 0;
	*memory[0x4015] = 0;
	bzero(memory[0x4000], 16);
	bzero(memory[0x4010], 4);
}

void cpu_irq()
{
	union u16 pc = {.value = __cpu_reg.pc};
	pc.value = __cpu_reg.pc += 2;
	writebus(0x01ff - __cpu_reg.sp--, pc.msb);
	writebus(0x01ff - __cpu_reg.sp--, pc.lsb);
	writebus(0x01ff - __cpu_reg.sp--, __cpu_reg.p.value);
	union u16 brk = readbus16(0xfffe);
	__cpu_reg.pc = brk.value;
	__cpu_reg.p.B = 0; // clear b flags
}

// global function
void reset(t_registers *reg, t_mem *memory)
{
	// https://www.pagetable.com/?p=410
	// TODO
}

int cpu_exec(t_mem *memory, t_registers *reg)
{

	uint8_t op = readbus(reg->pc);

#ifdef DEBUG_CPU
	if (op == 0)
	{
		// break;
		debug("DEBUG_CPU BREAK");
		return -1;
	}
#endif

	handle_op(_op[op]);

	// debug("cycle=%d", cycle);
	print_register(&__cpu_reg);

	return 0;
}

int cpu_run(void *pause)
{

	int debug = 0;
	int brk = 0;//0x0694;//0x0734;
	//int cpu_nolog_on_pc[] = {0x072f, 0x0730, 0x0731, 0x0732, -1};
	uint64_t t = (1000 * 1000 * 1000) / CPU_FREQ; // tick every 1ns // limit to 1Ghz
	const struct timespec time = {.tv_sec = CPU_FREQ == 1 ? 1 : 0, .tv_nsec = CPU_FREQ == 1 ? 0 : t};
	int quit = 0;
	// int log_cpu_set = !!(log_get_level_bin() | LOG_CPU | LOG_REGISTER | LOG_BUS);
	while (!quit)
	{

		while((*(char*)pause) == 1);

		// TODO: create a dedicated debugger function
		// And replace debug log with the name of instruction and value
		if (__cpu_reg.pc == brk)
			debug = 1;

		int i = 0;
		int flag = 0;
		// if (log_cpu_set) {
		// 	log_set_level_bin(log_get_level_bin() | LOG_CPU | LOG_REGISTER | LOG_BUS);
		// }
		// while(cpu_nolog_on_pc[i] != -1) {
		// 	if (cpu_nolog_on_pc[i] == __cpu_reg.pc) {
		// 		log_set_level_bin(log_get_level_bin() & ~(LOG_CPU | LOG_REGISTER | LOG_BUS)); // unset log_cpu
		// 		flag = true;
		// 		break;
		// 	}
		// 	i++;
		// }
		if (debug)
		{
			putchar('>');
			putchar(' ');
			fflush(stdout);
			int c = getchar();
			if (c == 'p')
			{
				// hexdumpSnake(*(__cpu_memory + 0x200), 1024);
				continue;
			}
			else if (c == 'r')
			{
				debug = 0;
				continue;
			}
			// lf 10
			quit = cpu_exec(__cpu_memory, &__cpu_reg);
			continue;
		}

		const int sleep = nanosleep(&time, NULL);
		quit = cpu_exec(__cpu_memory, &__cpu_reg);
	}

	return quit;
}

// used in test
void run(t_mem *memory, size_t size, t_registers *reg)
{

	debug("RUN 6502 with a memory of %zu octets", size);

	while (cpu_exec(memory, reg) != 0)
	{
	}
}
