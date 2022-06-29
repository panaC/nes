#include "cpu.h"

void adc(t_cpu* cpu, uint16_t addr) {

  cpu->registers.a += cpu->memory[addr] + (uint8_t) cpu->registers.p.C;

  check_flags_with_acc(&cpu->registers);
}

void and(t_cpu* cpu, uint16_t addr) {

  cpu->registers.a &= cpu->memory[addr];

  check_flags_with_acc(&cpu->registers);
}

void bcc(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.C == 0)
    cpu->registers.pc += jump;
}

void bcc(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.C == 1)
    cpu->registers.pc += jump;
}

void beq(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.Z == 1)
    cpu->registers.pc += jump;
}

void bit(t_cpu* cpu, uint16_t addr) {

  if (cpu->memory[addr] & cpu->registers.a == 0)
    cpu->registers.Z = 1;
  else
    cpu->registers.Z = 0;

  cpu->registers.N = ((uint8_t)addr & 0x80) >> 7;
  cpu->registers.V = ((uint8_t)addr & 0x40) >> 6;
}

void bmi(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.N == 1)
    cpu->registers.pc += jump;
}

void bne(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.Z == 0)
    cpu->registers.pc += jump;
}

void beq(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.Z == 1)
    cpu->registers.pc += jump;
}

void bpl(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.N == 0)
    cpu->registers.pc += jump;
}

void bvc(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.V == 0)
    cpu->registers.pc += jump;
}

void bvs(t_cpu* cpu, uint16_t jump) {

  if (cpu->registers.V == 1)
    cpu->registers.pc += jump;
}

void brk(t_cpu* cpu, uint16_t unused) {

  cpu->memory[cpu->registers.sp] = cpu->registers.pc.pch;
  --cpu->registers.sp;
  cpu->memory[cpu->registers.sp] = cpu->registers.pc.pcl;
  --cpu->registers.sp;
  cpu->memory[cpu->registers.sp] = cpu->registers.p;
  --cpu->registers.sp;

  cpu->registers.pc.value = 0xfffe;
}

void clc(t_cpu* cpu, uint16_t unused) {

  cpu->registers.p.C = 0;
}

void cld(t_cpu* cpu, uint16_t unused) {

  cpu->registers.p.D = 0;
}

void cli(t_cpu* cpu, uint16_t unused) {

  cpu->registers.p.I = 0;
}

void clv(t_cpu* cpu, uint16_t unused) {

  cpu->registers.p.V = 0;
}

void compare(t_registers* registers, uint16_t v1, uint16_t v2) {

  int8_t res = v1 - v2;

  if (acc == 0)
    registers->Z = 1;
  else
    registers->Z = 0;

  if ((acc & 0x80) >> 7 == 1)
    registers->N = 1;
  else
    registers->N = 0;

  if (acc >= 0)
    registers->C = 1;
  else
    registers->C = 0;
}

void cmp(t_cpu* cpu, uint16_t addr) {

  compare(registers, cpu->registers->a, cpu->memory[addr]);
}

void cpx(t_cpu* cpu, uint16_t addr) {

  compare(registers, cpu->registers->x, cpu->memory[addr]);
}

void cpy(t_cpu* cpu, uint16_t addr) {

  compare(registers, cpu->registers->y, cpu->memory[addr]);
}

void dec(t_cpu* cpu, uint16_t addr) {
  --cpu->memory[addr];
}

void dex(t_cpu* cpu, uint16_t addr) {
  --cpu->registers->x;
}

void dey(t_cpu* cpu, uint16_t addr) {
  --cpu->registers->y;
}

void eor(t_cpu* cpu, uint16_t addr) {

  cpu->registers->a ^= cpu->memory[addr];
}

