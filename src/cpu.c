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
void init(t_registers *reg, t_mem *memory)
{

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

uint32_t addressmode_zeropage(uint8_t arg) {
	return arg;
}

uint32_t addressmode_zeropagex(uint8_t arg) {
	return (arg + __cpu_reg.x) % 256;
}

uint32_t addressmode_zeropagey(uint8_t arg) {
	return (arg + __cpu_reg.x) % 256;
}

uint32_t addressmode_absolute(union u16 arg) {
	return arg.value;
}

uint32_t addressmode_absolutex(union u16 arg) {
	return arg.value + __cpu_reg.x;
}

uint32_t addressmode_absolutey(union u16 arg) {
	return arg.value + __cpu_reg.y;
}

uint32_t addressmode_indirectx(uint8_t arg) {
	return readbus((arg + __cpu_reg.x) % 256) + readbus((arg + __cpu_reg.x + 1) % 256) * 256;
}

uint32_t addressmode_indirecty(uint8_t arg) {
	return readbus(arg) + readbus((arg + 1) % 256) * 256 + __cpu_reg.y;
}

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
 * 
 * 
 * 
 * 
 * 
 */

void debug_opcode_accumulator(char *str, uint8_t op)
{
	debug("$%04x    %02x           %s", __cpu_reg.pc, op, str);
}

void debug_opcode_implied(char *str, uint8_t op)
{
	debug("$%04x    %02x           %s", __cpu_reg.pc, op, str);
}

void debug_opcode_immediate(char *str, uint8_t op, uint8_t arg)
{
	debug("$%04x    %02x %02x        %s #$%02x", __cpu_reg.pc, op, arg, str, arg);
}

void debug_opcode_absolute(char *str, uint8_t op, union u16 arg)
{
	debug("$%04x    %02x %02x %02x     %s $%04x", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
}

void debug_opcode_zeropage(char *str, uint8_t op, uint8_t arg)
{
	debug("$%04x    %02x %02x        %s $%02x", __cpu_reg.pc, op, arg, str, arg);
}

void debug_opcode_relative(char *str, uint8_t op, uint8_t arg)
{
	debug("$%04x    %02x %02x        %s $%02x", __cpu_reg.pc, op, arg, str, arg);
}

void debug_opcode_absolutex(char *str, uint8_t op, union u16 arg)
{
	debug("$%04x    %02x %02x %02x     %s $%04x,X", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
}

void debug_opcode_absolutey(char *str, uint8_t op, union u16 arg)
{
	debug("$%04x    %02x %02x %02x     %s $%04x,Y", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
}

void debug_opcode_zeropagex(char *str, uint8_t op, uint8_t arg)
{
	debug("$%04x    %02x %02x        %s $%02x,X", __cpu_reg.pc, op, arg, str, arg);
}

void debug_opcode_zeropagey(char *str, uint8_t op, uint8_t arg)
{
	debug("$%04x    %02x %02x        %s $%02x,Y", __cpu_reg.pc, op, arg, str, arg);
}

void debug_opcode_indirect(char *str, uint8_t op, union u16 arg)
{
	debug("$%04x    %02x %02x %02x     %s ($%04x)", __cpu_reg.pc, op, arg.lsb, arg.msb, str, arg.value);
}

void debug_opcode_indirectx(char *str, uint8_t op, uint8_t arg)
{
	debug("$%04x    %02x %02x        %s ($%02x,X)", __cpu_reg.pc, op, arg, str, arg);
}

void debug_opcode_indirecty(char *str, uint8_t op, uint8_t arg)
{
	debug("$%04x    %02x %02x        %s ($%02x,Y)", __cpu_reg.pc, op, arg, str, arg);
}

int adc_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	int16_t result;
	char *str = "ADC";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};

	switch (op)
	{
	case 0x69: // immediate
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		result = reg->a + arg + reg->p.C;

		reg->pc += 2;
		cycle += 2;

		break;
	case 0x65:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		result = reg->a + readbus(addressmode_zeropage(arg)) + reg->p.C;

		reg->pc += 2;
		cycle += 3;

		break;
	case 0x75:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		result = reg->a + readbus(addressmode_zeropagex(arg)) + reg->p.C;

		reg->pc += 2;
		cycle += 4;

		break;
	case 0x6d:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		result = reg->a + readbus(addressmode_absolute(uarg)) + reg->p.C;

		reg->pc += 3;
		cycle += 4;

		break;
	case 0x7d:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		result = reg->a + readbus(addressmode_absolutex(uarg)) + reg->p.C;

		reg->pc += 3;
		if ((uarg.value & 0x00ff) + reg->x > 0xff)
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
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		result = reg->a + readbus(addressmode_absolutey(uarg)) + reg->p.C;

		reg->pc += 3;
		if ((uarg.value & 0x00ff) + reg->x > 0xff)
			cycle += 5;
		else
			cycle += 4;

		break;

	case 0x61:
		arg = readbus_pc();
		debug_opcode_indirectx(str, op, arg);
		result = reg->a + readbus(addressmode_indirectx(arg)) + reg->p.C;

		reg->pc += 2;
		cycle += 6;

		break;
	case 0x71:
		arg = readbus_pc();
		debug_opcode_indirecty(str, op, arg);
		result = reg->a + addressmode_indirecty(arg) + reg->p.C;

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
	reg->p.N = !!(result & 0x80);

	// http://www.6502.org/tutorials/vflag.html
	// seems to be the right value : see in github nes emulation
	reg->p.C = !!(result & 0x100);
	// As stated above, the second purpose of the carry flag
	// is to indicate when the result of the
	// addition or subtraction is outside the range 0 to 255, specifically:
	// When the addition result is 0 to 255, the carry is cleared.
	// When the addition result is greater than 255, the carry is set.

	// http://www.6502.org/tutorials/vflag.html
	reg->p.V = result >= 0 ? (result & 0x100) >> 8 : !((result & 0x80) >> 7);

	return 1;
}

int and_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "AND";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};

	switch (op)
	{
	case 0x29:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		reg->a &= arg;

		cycle += 2;
		reg->pc += 2;
		break;
	case 0x25:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		reg->a &= readbus(addressmode_zeropage(arg));

		cycle += 3;
		reg->pc += 2;
		break;

	case 0x35:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		reg->a &= readbus(addressmode_zeropagex(arg));

		cycle += 4;
		reg->pc += 2;
		break;

	case 0x2d:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		reg->a &= readbus(addressmode_absolute(uarg));

		cycle += 4;
		reg->pc += 3;
		break;

	case 0x3d:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		reg->a &= readbus(addressmode_absolutex(uarg));

		cycle += 4; // Todo +1 if page crossed
		reg->pc += 3;
		break;

	case 0x39:
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		reg->a &= readbus(addressmode_absolutey(uarg));

		cycle += 4; // todo +1 if page crossed
		reg->pc += 3;
		break;

	case 0x21:
		arg = readbus_pc();
		debug_opcode_indirectx(str, op, arg);
		reg->a &= readbus(addressmode_indirectx(arg));

		cycle += 6;
		reg->pc += 2;
		break;

	case 0x31:
		arg = readbus_pc();
		debug_opcode_indirecty(str, op, arg);
		reg->a &= readbus(addressmode_indirecty(arg));

		cycle += 5; // todo +1 if page crossed
		reg->pc += 2;
		break;

	default:
		return 0;
	}
	reg->p.Z = !reg->a;
	reg->p.N = !!(reg->a & 0x80);

	return 1;
}

int asl_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "ASL";
	uint16_t result;

	uint8_t arg = 0;
	uint32_t addr = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0x0a:
		debug_opcode_accumulator(str, op);
		result = reg->a << 1;
		reg->a <<= 1;

		cycle += 2;
		reg->pc += 1;
		break;

	case 0x06:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		addr = addressmode_zeropage(arg);
		result = readbus(addr) << 1;
		writebus(addr, result);

		cycle += 5;
		reg->pc += 2;
		break;

	case 0x16:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		addr = addressmode_zeropagex(arg);
		result = readbus(addr) << 1;
		writebus(addr, result);

		cycle += 6;
		reg->pc += 2;
		break;

	case 0x0e:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		addr = addressmode_absolute(uarg);
		result = readbus(addr) << 1;
		writebus(addr, result);

		cycle += 6;
		reg->pc += 3;
		break;

	case 0x1e:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		addr = addressmode_absolutex(uarg);
		result = readbus(addr) << 1;
		writebus(addr, result);

		cycle += 7;
		reg->pc += 3;
		break;

	default:
		return 0;
	}

	reg->p.Z = !result;
	reg->p.N = !!(result & 0x80);
	reg->p.C = !!(result & 0x80); // Set to contents of old bit 7

	return 1;
}

int bcc_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0x90)
	{
		int8_t arg = readbus_pc();
		debug_opcode_relative("BCC", op, arg);
		if (reg->p.C == 0)
		{
			reg->pc += arg;
		}
		reg->pc += 2;
		cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
		return 1;
	}
	return 0;
}

int bcs_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0xb0)
	{
		int8_t arg = readbus_pc();
		debug_opcode_relative("BCS", op, arg);
		if (reg->p.C == 1)
		{
			reg->pc += arg;
		}
		reg->pc += 2;
		cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
		return 1;
	}
	return 0;
}

int beq_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0xf0)
	{
		int8_t arg = readbus_pc();
		debug_opcode_relative("BEQ", op, arg);
		if (reg->p.Z == 1)
		{
			reg->pc += arg;
		}
		reg->pc += 2; // in both case
		cycle += 2;	  // TODO (+1 if branch succeeds +2 if to a new page)
		return 1;
	}
	return 0;
}

int bit_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "BIT";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	int8_t value;
	switch (op)
	{
	case 0x24:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		value = reg->a & readbus(addressmode_zeropage(arg));

		reg->pc += 2;
		cycle += 3;
		break;

	case 0x2c:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		value = reg->a & readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4;
		break;

	default:
		return 0;
	}
	reg->p.Z = !value;
	reg->p.V = !!(value & 0x40);
	reg->p.N = !!(value & 0x80);

	return 1;
}

int bmi_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0x30)
	{
		int8_t arg = readbus_pc();
		debug_opcode_relative("BMI", op, arg);
		if (reg->p.N == 1)
		{
			reg->pc += arg;
		}
		reg->pc += 2;
		cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
		return 1;
	}
	return 0;
}

int bne_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0xd0)
	{
		int8_t arg = readbus_pc();
		debug_opcode_relative("BNE", op, arg);
		if (reg->p.Z == 0)
		{
			reg->pc += arg;
		}
		reg->pc += 2;
		cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
		return 1;
	}
	return 0;
}

int bpl_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0x10)
	{
		int8_t arg = readbus_pc();
		debug_opcode_relative("BPL", op, arg);
		if (reg->p.N == 0)
		{
			reg->pc += arg;
		}
		reg->pc += 2;
		cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
		return 1;
	}
	return 0;
}

int bvc_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0x50)
	{
		int8_t arg = readbus_pc();
		debug_opcode_relative("BVC", op, arg);

		if (reg->p.V == 0)
		{
			reg->pc += arg;
		}
		reg->pc += 2;
		cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
		return 1;
	}
	return 0;
}

int bvs_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0x70)
	{
		int8_t arg = readbus_pc();
		debug_opcode_relative("BVS", op, arg);

		if (reg->p.V == 1)
		{
			reg->pc += arg;
		}
		reg->pc += 2;
		cycle += 2; // TODO (+1 if branch succeeds +2 if to a new page)
		return 1;
	}
	return 0;
}

int clr_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	switch (op)
	{
	case 0x18:
		debug_opcode_implied("CLC", op);
		// clc
		reg->p.C = 0;
		break;

	case 0xd8:
		debug_opcode_implied("CLD", op);
		// cld
		reg->p.D = 0;
		break;

	case 0x58:
		debug_opcode_implied("CLI", op);
		// cli
		reg->p.I = 0;
		break;

	case 0xb8:
		debug_opcode_implied("CLV", op);
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

int sbc_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "SBC";

	int16_t result;
	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xe9:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		result = reg->a - arg - (1 - reg->p.C);

		reg->pc += 2;
		cycle += 2;
		break;

	case 0xe5:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		result = reg->a - readbus(addressmode_zeropage(arg)) - (1 - reg->p.C);

		reg->pc += 2;
		cycle += 3;
		break;

	case 0xf5:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		result = reg->a - readbus(addressmode_zeropagex(arg)) - (1 - reg->p.C);

		reg->pc += 2;
		cycle += 4;
		break;

	case 0xed:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		result = reg->a - readbus(addressmode_absolute(uarg)) - (1 - reg->p.C);

		reg->pc += 3;
		cycle += 4;
		break;

	case 0xfd:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		result = reg->a - readbus(addressmode_absolutex(uarg)) - (1 - reg->p.C);

		reg->pc += 3;
		cycle += 4; // TODO: +1 if page crossed
		break;

	case 0xf9:
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		result = reg->a - readbus(addressmode_absolutey(uarg)) - (1 - reg->p.C);

		reg->pc += 3;
		cycle += 4; // TODO: +1 if page crossed
		break;

	case 0xe1:
		arg = readbus_pc();
		debug_opcode_indirectx(str, op, arg);
		result = reg->a - readbus(addressmode_indirectx(arg)) - (1 - reg->p.C);

		reg->pc += 2;
		cycle += 6;
		break;

	case 0xf1:
		arg = readbus_pc();
		debug_opcode_indirecty(str, op, arg);
		result = reg->a - readbus(addressmode_indirecty(arg)) - (1 - reg->p.C);
		reg->pc += 2;
		cycle += 6; // TODO: +1 if page crossed
		break;

	default:
		return 0;
	}

	reg->a = result;

	reg->p.Z = !result;
	reg->p.N = !!(result & 0x80);

	// http://www.6502.org/tutorials/vflag.html
	reg->p.C = !(!!(result & 0x100));
	// As stated above, the second purpose of the carry flag
	// is to indicate when the result of the
	// addition or subtraction is outside the range 0 to 255, specifically:
	// When the subtraction result is 0 to 255, the carry is set.
	// When the subtraction result is less than 0, the carry is cleared.

	// http://www.6502.org/tutorials/vflag.html
	reg->p.V = result >= 0 ? (result & 0x100) >> 8 : !((result & 0x80) >> 7);

	return 1;
}

int set_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	switch (op)
	{
	case 0x38:
		debug_opcode_implied("SEC", op);
		// sec
		reg->p.C = 1;
		break;

	case 0xf8:
		// sed
		debug_opcode_implied("SED", op);
		reg->p.D = 1;
		break;

	case 0x78:
		// sei
		debug_opcode_implied("SEI", op);
		reg->p.I = 1;
		break;

	default:
		return 0;
	}

	reg->pc += 1;
	cycle += 2;
	return 1;
}

int trs_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	int8_t result;
	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xaa:
		debug_opcode_implied("TAX", op);
		// tax
		reg->x = reg->a;
		reg->p.Z = !reg->x;
		reg->p.N = (reg->x & 0x80) >> 7;
		break;

	case 0xa8:
		debug_opcode_implied("TAY", op);
		// tay
		reg->y = reg->a;
		reg->p.Z = !reg->y;
		reg->p.N = (reg->y & 0x80) >> 7;
		break;

	case 0xba:
		debug_opcode_implied("TSX", op);
		// tsx
		reg->x = reg->sp;
		reg->p.Z = !reg->x;
		reg->p.N = (reg->x & 0x80) >> 7;
		break;

	case 0x8a:
		debug_opcode_implied("TXA", op);
		// txa
		reg->a = reg->x;
		reg->p.Z = !reg->a;
		reg->p.N = (reg->a & 0x80) >> 7;
		break;

	case 0x9a:
		debug_opcode_implied("TXS", op);
		// txs
		reg->sp = reg->x;
		// no check processor status
		break;

	case 0x98:
		debug_opcode_implied("TYA", op);
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

int cmp_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "CMP";

	int8_t mem;
	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xc9:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		mem = arg;

		reg->pc += 2;
		cycle += 2;
		break;

	case 0xc5:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		mem = readbus(addressmode_zeropage(arg));

		reg->pc += 2;
		cycle += 3;
		break;

	case 0xd5:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		mem = readbus(addressmode_zeropagex(arg));

		reg->pc += 2;
		cycle += 4;
		break;

	case 0xcd:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		mem = readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4;
		break;

	case 0xdd:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		mem = readbus(addressmode_absolutex(uarg));

		reg->pc += 3;
		cycle += 4; // TODO +1 if page crossed
		break;

	case 0xd9:
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		mem = readbus(addressmode_absolutey(uarg));

		reg->pc += 3;
		cycle += 4; // TODO +1 if page crossed
		break;

	case 0xc1:
		arg = readbus_pc();
		debug_opcode_indirectx(str, op, arg);
		mem = readbus(addressmode_indirectx(arg));

		reg->pc += 2;
		cycle += 6;
		break;

	case 0xd1:
		arg = readbus_pc();
		debug_opcode_indirecty(str, op, arg);
		mem = readbus(addressmode_indirecty(arg));

		reg->pc += 2;
		cycle += 5; // TODO +1 if page crossed
		break;

	default:
		return 0;
	}

	reg->p.Z = (int8_t)reg->a == mem ? 1 : 0;
	reg->p.N = (int8_t)reg->a < mem ? 1 : 0;
	reg->p.C = (int8_t)reg->a >= mem ? 1 : 0;

	return 1;
}

int cpx_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "CPX";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	int8_t mem;
	switch (op)
	{
	case 0xe0:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		mem = arg;

		reg->pc += 2;
		cycle += 2;
		break;

	case 0xe4:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		mem = readbus(addressmode_zeropage(arg));

		reg->pc += 2;
		cycle += 3;
		break;

	case 0xec:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		mem = readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4;
		break;

	default:
		return 0;
	}

	reg->p.Z = (int8_t)reg->x == mem ? 1 : 0;
	reg->p.N = (int8_t)reg->x < mem ? 1 : 0;
	reg->p.C = (int8_t)reg->x >= mem ? 1 : 0;

	return 1;
}

int cpy_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "CPY";

	int8_t mem;
	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xc0:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		mem = arg;

		reg->pc += 2;
		cycle += 2;
		break;

	case 0xc4:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		mem = readbus(addressmode_zeropage(arg));

		reg->pc += 2;
		cycle += 3;
		break;

	case 0xcc:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		mem = readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4;
		break;

	default:
		return 0;
	}

	reg->p.Z = (int8_t)reg->y == mem ? 1 : 0;
	reg->p.N = (int8_t)reg->y < mem ? 1 : 0;
	reg->p.C = (int8_t)reg->y >= mem ? 1 : 0;

	return 1;
}

int dec_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "DEC";

	int8_t result;
	uint8_t arg = 0;
	uint32_t addr = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xc6:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		addr = addressmode_zeropage(arg);
		result = (int8_t)readbus(addr) - 1;
		writebus(addr, result);

		reg->pc += 2;
		cycle += 5;
		break;

	case 0xd6:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		addr = addressmode_zeropagex(arg);
		result = (int8_t)readbus(addr) - 1;
		writebus(addr, result);

		reg->pc += 2;
		cycle += 6;

	case 0xce:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		addr = addressmode_absolute(uarg);
		result = (int8_t)readbus(addr) - 1;
		writebus(addr, result);

		reg->pc += 3;
		cycle += 6;

	case 0xde:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		addr = addressmode_absolutex(uarg);
		result = (int8_t)readbus(addr) - 1;
		writebus(addr, result);

		reg->pc += 3;
		cycle += 7;

	default:
		return 0;
	}

	reg->p.Z = !result; 
	reg->p.N = !!(result & 0x80);

	return 1;
}

int inc_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "INC";

	int8_t result;
	uint32_t addr = 0;
	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xe6:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		addr = addressmode_zeropage(arg);
		result = (int8_t)readbus(addr) + 1;
		writebus(addr, result);

		reg->pc += 2;
		cycle += 5;
		break;

	case 0xf6:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		addr = addressmode_zeropagex(arg);
		result = (int8_t)readbus(addr) + 1;
		writebus(addr, result);

		reg->pc += 2;
		cycle += 6;

	case 0xee:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		addr = addressmode_absolute(uarg);
		result = (int8_t)readbus(addr) + 1;
		writebus(addr, result);

		reg->pc += 3;
		cycle += 6;

	case 0xfe:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		addr = addressmode_absolutex(uarg);
		result = (int8_t)readbus(addr) + 1;
		writebus(addr, result);

		reg->pc += 3;
		cycle += 7;

	default:
		return 0;
	}

	reg->p.Z = !result;
	reg->p.N = !!(result & 0x80);

	return 1;
}

int dex_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0xca)
	{
		debug_opcode_implied("DEX", op);
		reg->x = (reg->x - 1) & 0xff;
		reg->p.Z = !reg->x;
		reg->p.N = !!(reg->x & 0x80);
		reg->pc += 1;
		cycle += 2;
		return 1;
	}
	return 0;
}

int dey_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0x88)
	{
		debug_opcode_implied("DEY", op);
		reg->y = (reg->y - 1) & 0xff;
		reg->p.Z = !reg->y;
		reg->p.N = !!(reg->y & 0x80);
		reg->pc += 1;
		cycle += 2;
		return 1;
	}
	return 0;
}

int inx_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0xe8)
	{
		debug_opcode_implied("INX", op);
		reg->x = (reg->x + 1) & 0xff;
		reg->p.Z = !reg->x;
		reg->p.N = !!(reg->x & 0x80);
		reg->pc += 1;
		cycle += 2;
		return 1;
	}
	return 0;
}

int iny_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0xc8)
	{
		debug_opcode_implied("INY", op);
		reg->y = (reg->y + 1) & 0xff;
		reg->p.Z = !reg->y;
		reg->p.N = !!(reg->y & 0x80);
		reg->pc += 1;
		cycle += 2;
		return 1;
	}
	return 0;
}

int eor_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "EOR";

	int8_t result;
	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0x49:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		result = reg->a ^ arg;

		reg->pc += 2;
		cycle += 2;
		break;

	case 0x45:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		result = reg->a ^ (int8_t)readbus(addressmode_zeropage(arg));

		reg->pc += 2;
		cycle += 3;
		break;

	case 0x55:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		result = reg->a ^ (int8_t)readbus(addressmode_zeropagex(arg));

		reg->pc += 2;
		cycle += 4;
		break;

	case 0x4d:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		result = reg->a ^ (int8_t)readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4;
		break;

	case 0x5d:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		result = reg->a ^ (int8_t)readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4; // TODO +1 if page crossed
		break;

	case 0x59:
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		result = reg->a ^ (int8_t)readbus(addressmode_absolutey(uarg));

		reg->pc += 3;
		cycle += 4; // TODO +1 if page crossed
		break;

	case 0x41:
		arg = readbus_pc();
		debug_opcode_indirectx(str, op, arg);
		result = reg->a ^ (int8_t)readbus(addressmode_indirectx(arg));

		reg->pc += 2;
		cycle += 6;
		break;

	case 0x51:
		arg = readbus_pc();
		debug_opcode_indirecty(str, op, arg);
		result = reg->a ^ (int8_t)readbus(addressmode_indirecty(arg));

		reg->pc += 2;
		cycle += 5; // TODO +1 if page crossed
		break;

	default:
		return 0;
	}

	reg->a = result;

	reg->p.Z = !result;
	reg->p.N = !!(result & 0x80);

	return 1;
}

int jmp_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "JMP";

	union u16 uarg = {0};
	switch (op)
	{
	case 0x4c:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		reg->pc = uarg.value;
		cycle += 3;
		break;

	case 0x6c:
		uarg = readbus16_pc();
		debug_opcode_indirect(str, op, uarg);
		assert(0);
		reg->pc += 3; // TODO: how to do indirect jump ? https://www.nesdev.org/obelisk-6502-guide/addressing.html#IND
		cycle += 5;
		break;

	default:
		return 0;
	}

	return 1;
}

int lda_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "LDA";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xa9:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		reg->a = arg;

		reg->pc += 2;
		cycle += 2;
		break;

	case 0xa5:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		reg->a = readbus(addressmode_zeropage(arg));

		reg->pc += 2;
		cycle += 3;
		break;

	case 0xb5:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		reg->a = readbus(addressmode_zeropagex(arg));

		reg->pc += 2;
		cycle += 4;
		break;

	case 0xad:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		reg->a = readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4;
		break;

	case 0xbd:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		reg->a = readbus(addressmode_absolutex(uarg));

		reg->pc += 3;
		cycle += 4; // TODO +1 if page crossed
		break;

	case 0xb9:
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		reg->a = readbus(addressmode_absolutey(uarg));

		reg->pc += 3;
		cycle += 4; // TODO +1 if page crossed
		break;

	case 0xa1:
		arg = readbus_pc();
		debug_opcode_indirectx(str, op, arg);
		reg->a = readbus(addressmode_indirectx(arg));

		reg->pc += 2;
		cycle += 6;
		break;

	case 0xb1:
		arg = readbus_pc();
		debug_opcode_indirecty(str, op, arg);
		reg->a = readbus(addressmode_indirecty(arg));

		reg->pc += 2;
		cycle += 5; // TODO +1 if page crossed
		break;

	default:
		return 0;
	}

	reg->p.Z = !reg->a; 
	reg->p.N = !!(reg->a & 0x80);

	return 1;
}

int ldx_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "LDX";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xa2:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		reg->x = arg;

		reg->pc += 2;
		cycle += 2;
		break;

	case 0xa6:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		reg->x = readbus(addressmode_zeropage(arg));

		reg->pc += 2;
		cycle += 3;
		break;

	case 0xb6:
		arg = readbus_pc();
		debug_opcode_zeropagey(str, op, arg);
		reg->x = readbus(addressmode_zeropagey(arg));

		reg->pc += 2;
		cycle += 4;
		break;

	case 0xae:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		reg->x = readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4;
		break;

	case 0xbe:
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		reg->x = readbus(addressmode_absolutey(uarg));

		reg->pc += 3;
		cycle += 4; // TODO +1 if page crossed
		break;

	default:
		return 0;
	}

	reg->p.Z = !reg->x;
	reg->p.N = !!(reg->x & 0x80);

	return 1;
}

int ldy_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "LDY";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0xa0:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		reg->y = arg;

		reg->pc += 2;
		cycle += 2;
		break;

	case 0xa4:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		reg->y = readbus(addressmode_zeropage(arg));

		reg->pc += 2;
		cycle += 3;
		break;

	case 0xb4:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		reg->y = readbus(addressmode_zeropagex(arg));

		reg->pc += 2;
		cycle += 4;
		break;

	case 0xac:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		reg->y = readbus(addressmode_absolute(uarg));

		reg->pc += 3;
		cycle += 4;
		break;

	case 0xbc:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		reg->y = readbus(addressmode_absolutex(uarg));

		reg->pc += 3;
		cycle += 4; // TODO +1 if page crossed
		break;

	default:
		return 0;
	}

	reg->p.Z = !reg->y;
	reg->p.N = !!(reg->y & 0x80);

	return 1;
}

int lsr_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "LSR";

	uint16_t result;
	uint32_t addr;
	uint8_t arg = 0;
	uint8_t mem = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0x4a:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		mem = reg->a;
		result = mem >> 1;
		reg->a >>= 1;

		cycle += 2;
		reg->pc += 1;
		break;

	case 0x46:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		addr = addressmode_zeropage(arg);
		mem = readbus(addr);
		result = mem >> 1;
		writebus(addr, result);

		cycle += 5;
		reg->pc += 2;
		// TODO
		break;

	case 0x56:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		addr = addressmode_zeropagex(arg);
		mem = readbus(addr);
		result = mem >> 1;
		writebus(addr, result);

		cycle += 6;
		reg->pc += 2;
		break;

	case 0x4e:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		addr = addressmode_absolute(uarg);
		mem = readbus(addr);
		result = mem >> 1;
		writebus(addr, result);

		cycle += 6;
		reg->pc += 3;
		break;

	case 0x5e:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		addr = addressmode_absolutex(uarg);
		mem = readbus(addr);
		result = mem >> 1;
		writebus(addr, result);

		cycle += 7;
		reg->pc += 3;
		break;

	default:
		return 0;
	}

	reg->p.Z = !result;
	reg->p.N = !!(result & 0x80);
	reg->p.C = mem & 0x1; // 	Set to contents of old bit 0

	return 1;
}

int ora_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "ORA";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0x09:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		reg->a |= arg;

		cycle += 2;
		reg->pc += 2;
		break;
	case 0x05:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		reg->a |= readbus(addressmode_zeropage(arg));

		cycle += 3;
		reg->pc += 2;
		break;

	case 0x15:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		reg->a |= readbus(addressmode_zeropagex(arg));

		cycle += 4;
		reg->pc += 2;
		break;

	case 0x0d:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		reg->a |= readbus(addressmode_absolute(uarg));

		cycle += 4;
		reg->pc += 3;
		break;

	case 0x1d:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		reg->a |= readbus(addressmode_absolutex(uarg));

		cycle += 4; // Todo +1 if page crossed
		reg->pc += 3;
		break;

	case 0x19:
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		reg->a |= readbus(addressmode_absolutey(uarg));

		cycle += 4; // todo +1 if page crossed
		reg->pc += 3;
		break;

	case 0x01:
		arg = readbus_pc();
		debug_opcode_indirectx(str, op, arg);
		reg->a |= readbus(addressmode_indirectx(arg));

		cycle += 6;
		reg->pc += 2;
		break;

	case 0x11:
		arg = readbus_pc();
		debug_opcode_indirecty(str, op, arg);
		reg->a |= readbus(addressmode_indirecty(arg));

		cycle += 5; // todo +1 if page crossed
		reg->pc += 2;
		break;

	default:
		return 0;
	}

	reg->p.Z = !reg->a;
	reg->p.N = !!(reg->a & 0x80);
	return 1;
}

int sta_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "STA";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0x85:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		writebus(addressmode_zeropage(arg), reg->a);

		cycle += 3;
		reg->pc += 2;
		break;

	case 0x95:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		writebus(addressmode_zeropagex(arg), reg->a);

		cycle += 4;
		reg->pc += 2;
		break;

	case 0x8d:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		writebus(addressmode_absolute(uarg), reg->a);

		cycle += 4;
		reg->pc += 3;
		break;

	case 0x9d:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		writebus(addressmode_absolute(uarg), reg->a);

		cycle += 5;
		reg->pc += 3;
		break;

	case 0x99:
		uarg = readbus16_pc();
		debug_opcode_absolutey(str, op, uarg);
		writebus(addressmode_absolutey(uarg), reg->a);

		cycle += 5;
		reg->pc += 3;
		break;

	case 0x81:
		arg = readbus_pc();
		debug_opcode_indirectx(str, op, arg);
		writebus(addressmode_indirectx(arg), reg->a);

		cycle += 6;
		reg->pc += 2;
		break;

	case 0x91:
		arg = readbus_pc();
		debug_opcode_indirecty(str, op, arg);
		writebus(addressmode_indirecty(arg), reg->a);

		cycle += 6;
		reg->pc += 2;
		break;

	default:
		return 0;
	}

	return 1;
}

int stx_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "STX";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0x86:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		writebus(addressmode_zeropage(arg), reg->x);

		cycle += 3;
		reg->pc += 2;
		break;
	case 0x96:
		arg = readbus_pc();
		debug_opcode_zeropagey(str, op, arg);
		writebus(addressmode_zeropagey(arg), reg->x);

		cycle += 4;
		reg->pc += 2;
		break;

	case 0x8e:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		writebus(addressmode_absolute(uarg), reg->x);

		cycle += 4;
		reg->pc += 3;
		break;

	default:
		return 0;
	}

	return 1;
}

int sty_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "STY";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	switch (op)
	{
	case 0x84:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		writebus(addressmode_zeropage(arg), reg->y);

		cycle += 3;
		reg->pc += 2;
		break;

	case 0x94:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		writebus(addressmode_zeropagex(arg), reg->y);

		cycle += 4;
		reg->pc += 2;
		break;

	case 0x8c:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		writebus(addressmode_absolute(uarg), reg->y);

		cycle += 4;
		reg->pc += 3;
		break;

	default:
		return 0;
	}

	return 1;
}

int rol_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "ROL";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	uint32_t addr = 0;
	uint8_t mem, value, oldbit = 0;
	switch (op)
	{
	case 0x2a:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		oldbit = (reg->a & 0x80) >> 7;
		reg->a <<= 1;
		reg->a = reg->a | reg->p.C;
		reg->p.C = oldbit;

		reg->pc += 1;
		cycle += 2;
		break;

	case 0x26:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		addr = addressmode_zeropage(arg);
		mem = readbus(arg);
		oldbit = (mem & 0x80) >> 7;
		reg->a <<= 1;
		value = mem | reg->p.C;
		writebus(addr, value);
		reg->p.C = oldbit;

		reg->pc += 2;
		cycle += 5;
		break;

	case 0x36:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		addr = addressmode_zeropagex(arg);
		mem = readbus(arg);
		oldbit = (mem & 0x80) >> 7;
		reg->a <<= 1;
		value = mem | reg->p.C;
		writebus(addr, value);
		reg->p.C = oldbit;

		reg->pc += 2;
		cycle += 6;
		break;

	case 0x2e:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		addr = addressmode_absolute(uarg);
		mem = readbus(arg);
		oldbit = (mem & 0x80) >> 7;
		reg->a <<= 1;
		value = mem | reg->p.C;
		writebus(addr, value);
		reg->p.C = oldbit;

		reg->pc += 3;
		cycle += 6;
		break;

	case 0x3e:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		addr = addressmode_absolutex(uarg);
		mem = readbus(arg);
		oldbit = (mem & 0x80) >> 7;
		reg->a <<= 1;
		value = mem | reg->p.C;
		writebus(addr, value);
		reg->p.C = oldbit;
		reg->pc += 3;
		cycle += 7;
		break;

	default:
		return 0;
	}

	reg->p.Z = !value;
	reg->p.N = !!(value & 0x80);

	return 1;
}

int ror_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	char *str = "ROR";

	uint8_t arg = 0;
	union u16 uarg = {.value = 0};
	uint32_t addr = 0;
	uint8_t mem, value, oldbit = 0;
	switch (op)
	{
	case 0x6a:
		arg = readbus_pc();
		debug_opcode_immediate(str, op, arg);
		oldbit = reg->a & 0x01;
		reg->a >>= 1;
		value = reg->a |= reg->p.C << 7;
		reg->p.C = oldbit;

		reg->pc += 1;
		cycle += 2;
		break;

	case 0x66:
		arg = readbus_pc();
		debug_opcode_zeropage(str, op, arg);
		addr = addressmode_zeropage(arg);
		mem = readbus(addr);
		oldbit = mem & 0x01;
		reg->a >>= 1;
		value = mem | (reg->p.C << 7);
		writebus(addr, value);
		reg->p.C = oldbit;

		reg->pc += 2;
		cycle += 5;
		break;

	case 0x76:
		arg = readbus_pc();
		debug_opcode_zeropagex(str, op, arg);
		addr = addressmode_zeropagex(arg);
		mem = readbus(addr);
		oldbit = mem & 0x01;
		reg->a >>= 1;
		value = mem | (reg->p.C << 7);
		writebus(addr, value);
		reg->p.C = oldbit;

		reg->pc += 2;
		cycle += 6;
		break;

	case 0x6e:
		uarg = readbus16_pc();
		debug_opcode_absolute(str, op, uarg);
		addr = addressmode_absolute(uarg);
		mem = readbus(addr);
		oldbit = mem & 0x01;
		reg->a >>= 1;
		value = mem | (reg->p.C << 7);
		writebus(addr, value);
		reg->p.C = oldbit;

		reg->pc += 3;
		cycle += 6;
		break;

	case 0x7e:
		uarg = readbus16_pc();
		debug_opcode_absolutex(str, op, uarg);
		addr = addressmode_absolutex(uarg);
		mem = readbus(addr);
		oldbit = mem & 0x01;
		reg->a >>= 1;
		value = mem | (reg->p.C << 7);
		writebus(addr, value);
		reg->p.C = oldbit;

		reg->pc += 3;
		cycle += 7;
		break;

	default:
		return 0;
	}

	reg->p.Z = !value;
	reg->p.N = !!(value & 0x80);

	return 1;
	;
}

int jsr_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	union u16 pc = {0};
	if (op == 0x20)
	{
		union u16 uarg = readbus16_pc();
		debug_opcode_absolute("JSR", op, uarg);
		pc.value = reg->pc += 3;
		writebus(0x01ff - reg->sp, pc.msb);
		reg->sp--;
		writebus(0x01ff - reg->sp, pc.lsb);
		reg->sp--;
		reg->pc = uarg.value;
		cycle += 6;
		return 1;
	}

	return 0;
}

int rts_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	union u16 pc = {0};
	if (op == 0x60)
	{
		debug_opcode_implied("RTS", op);
		reg->sp++;
		pc.lsb = readbus(0x01ff - reg->sp);
		reg->sp++;
		pc.msb = readbus(0x01ff - reg->sp);
		reg->pc = pc.value;
		cycle += 6;
		return 1;
	}

	return 0;
}

int psp_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	switch (op)
	{
	case 0x48:
		debug_opcode_implied("PHA", op);
		writebus(0x01ff - reg->sp, reg->a);
		reg->sp--;
		reg->pc += 1;
		cycle += 3;
		break;

	case 0x08:
		debug_opcode_implied("PHP", op);
		writebus(0x01ff - reg->sp, reg->p.value);
		reg->sp--;
		reg->pc += 1;
		cycle += 3;
		break;

	case 0x68:
		debug_opcode_implied("PLA", op);
		reg->sp++;
		reg->a = readbus(0x01ff - reg->sp);
		reg->p.Z = !reg->a;
		reg->p.N = (reg->a & 0x80) >> 7;
		reg->pc += 1;
		cycle += 3;
		break;

	case 0x28:
		debug_opcode_implied("PLP", op);
		reg->sp++;
		reg->p.value = readbus(0x01ff - reg->sp);
		reg->pc += 1;
		cycle += 4;
		break;

	default:
		return 0;
	}

	return 1;
}

int brk_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{

	if (op == 0)
	{
		debug_opcode_implied("BRK", op);
		irq(reg, memory);
		reg->p.B = 1;

		return 1;
	}
	return 0;
}

int nop_opcode(t_registers *reg, t_mem *memory, uint8_t op)
{
	if (op == 0xea)
	{
		debug_opcode_implied("NOP", op);
		cycle += 2;
		reg->pc += 1;
		return 1;
	}
	return 0;
}

// global function
void reset(t_registers *reg, t_mem *memory)
{
	// https://www.pagetable.com/?p=410
	// TODO
}

void irq(t_registers *reg, t_mem *memory)
{
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

int cpu_exec(t_mem *memory, t_registers *reg)
{

	int res = 0;
	uint8_t op = readbus(reg->pc);
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
	res += adc_opcode(reg, memory, op);
	res += and_opcode(reg, memory, op);
	res += asl_opcode(reg, memory, op);
	res += bcc_opcode(reg, memory, op);
	res += bcs_opcode(reg, memory, op);
	res += beq_opcode(reg, memory, op);
	res += bit_opcode(reg, memory, op);
	res += bmi_opcode(reg, memory, op);
	res += bne_opcode(reg, memory, op);
	res += bpl_opcode(reg, memory, op);
	res += bvc_opcode(reg, memory, op);
	res += bvs_opcode(reg, memory, op);
	res += clr_opcode(reg, memory, op); // clear opcodes
	res += set_opcode(reg, memory, op); // set opcodes
	res += trs_opcode(reg, memory, op); // transfer opcodes
	res += cmp_opcode(reg, memory, op);
	res += cpx_opcode(reg, memory, op);
	res += cpy_opcode(reg, memory, op);
	res += dec_opcode(reg, memory, op);
	res += inc_opcode(reg, memory, op);
	res += dex_opcode(reg, memory, op);
	res += dey_opcode(reg, memory, op);
	res += inx_opcode(reg, memory, op);
	res += iny_opcode(reg, memory, op);
	res += eor_opcode(reg, memory, op);
	res += jmp_opcode(reg, memory, op);
	res += jsr_opcode(reg, memory, op);
	res += lda_opcode(reg, memory, op);
	res += ldx_opcode(reg, memory, op);
	res += ldy_opcode(reg, memory, op);
	res += lsr_opcode(reg, memory, op);
	res += ora_opcode(reg, memory, op);
	res += sta_opcode(reg, memory, op);
	res += stx_opcode(reg, memory, op);
	res += sty_opcode(reg, memory, op);
	res += rol_opcode(reg, memory, op);
	res += ror_opcode(reg, memory, op);
	res += rts_opcode(reg, memory, op);
	res += psp_opcode(reg, memory, op);
	res += sbc_opcode(reg, memory, op);
	res += nop_opcode(reg, memory, op);
	assert(res < 2);
	if (res)
	{
		// found
	}
	else
	{
		debug("OP=%x not found", op);
		reg->pc++;
	}

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
				// hexdumpSnake(*(__memory + 0x200), 1024);
				continue;
			}
			else if (c == 'r')
			{
				debug = 0;
				continue;
			}
			// lf 10
			quit = cpu_exec(__memory, &__cpu_reg);
			continue;
		}

		const int sleep = nanosleep(&time, NULL);
		quit = cpu_exec(__memory, &__cpu_reg);
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
