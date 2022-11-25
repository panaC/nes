#include <stdint.h>
#include <stdio.h>
#include "cpu.h"

static void debug_opcode(enum e_addressMode mode, char *str, uint8_t op, union u16 arg, uint32_t ptr) {

	switch (mode)
	{
	case ACCUMULATOR:
		printf("$%04x    %02x           %s\n", ptr, op, str);
		break;
	case IMPLIED:
		printf("$%04x    %02x           %s\n", ptr, op, str);
		break;
	case IMMEDIATE:
		printf("$%04x    %02x %02x        %s #$%02x\n", ptr, op, arg.lsb, str, arg.lsb);
		break;
	case ABSOLUTE:
		printf("$%04x    %02x %02x %02x     %s $%04x\n", ptr, op, arg.lsb, arg.msb, str, arg.value);
		break;
	case ZEROPAGE:
		printf("$%04x    %02x %02x        %s $%02x\n", ptr, op, arg.lsb, str, arg.lsb);
		break;
	case RELATIVE:
		printf("$%04x    %02x %02x        %s $%02x\n", ptr, op, arg.lsb, str, arg.lsb);
		break;
	case ABSOLUTEX:
		printf("$%04x    %02x %02x %02x     %s $%04x,X\n", ptr, op, arg.lsb, arg.msb, str, arg.value);
		break;
	case ABSOLUTEY:
		printf("$%04x    %02x %02x %02x     %s $%04x,Y\n", ptr, op, arg.lsb, arg.msb, str, arg.value);
		break;
	case ZEROPAGEX:
		printf("$%04x    %02x %02x        %s $%02x,X\n", ptr, op, arg.lsb, str, arg.lsb);
		break;
	case ZEROPAGEY:
		printf("$%04x    %02x %02x        %s $%02x,Y\n", ptr, op, arg.lsb, str, arg.lsb);
		break;
	case INDIRECT:
		printf("$%04x    %02x %02x %02x     %s ($%04x)\n", ptr, op, arg.lsb, arg.msb, str, arg.value);
		break;
	case INDIRECTX:
		printf("$%04x    %02x %02x        %s ($%02x,X)\n", ptr, op, arg.lsb, str, arg.lsb);
		break;
	case INDIRECTY:
		printf("$%04x    %02x %02x        %s ($%02x,Y)\n", ptr, op, arg.lsb, str, arg.lsb);
		break;

	default:
		break;
	}

}

static union u16 read_arg(enum e_addressMode mode, uint8_t *data, uint32_t ptr) {

	switch (mode)
	{
	case ACCUMULATOR:
		return (union u16){.value = 0};
	case IMPLIED:
		return (union u16){.value = 0};
	case IMMEDIATE:
		return (union u16){.value = data[ptr + 1]};
	case ABSOLUTE:
		return (union u16){.lsb = data[ptr + 1], .msb = data[ptr + 2]};
	case ZEROPAGE:
		return (union u16){.value = data[ptr + 1]};
	case RELATIVE:
		return (union u16){.value = data[ptr + 1]};
	case ABSOLUTEX:
		return (union u16){.lsb = data[ptr + 1], .msb = data[ptr + 2]};
	case ABSOLUTEY:
		return (union u16){.lsb = data[ptr + 1], .msb = data[ptr + 2]};
	case ZEROPAGEX:
		return (union u16){.value = data[ptr + 1]};
	case ZEROPAGEY:
		return (union u16){.value = data[ptr + 1]};
	case INDIRECT:
		return (union u16){.lsb = data[ptr + 1], .msb = data[ptr + 2]};
	case INDIRECTX:
		return (union u16){.value = data[ptr + 1]};
	case INDIRECTY:
		return (union u16){.value = data[ptr + 1]};
	}

	return (union u16){.value = 0};
}

void listing(uint8_t *data, uint32_t size) {

  uint32_t ptr = 0;

  cpu_init_op_tab();

  while(size > ptr) {
    struct instruction op = _op[data[ptr]];
    union u16 uarg = read_arg(op.mode, data, ptr);
    if (op.code != 0)
      debug_opcode(op.mode, op.str, op.code, uarg, ptr);
    ptr += op.size;
  }

}