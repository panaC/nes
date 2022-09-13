#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "test.h"
#include "utils.h"

void test_adc_opcode_immediate(t_mem *mem)
{

  t_registers r = {
      .pc = 0,
      .sp = 0,
      .p = 0,
      .a = 0,
      .x = 0,
      .y = 0};
  
  *mem[0] = 0x69; // adc immediate
  *mem[1] = 0x42;

  run(mem, MEM_SIZE, &r);

  assert(r.a == 0x42);
  assert(r.pc == 2);
  assert(r.p.C == 0); // no carry

  hexdump(*mem, 16);

  printf("test: ADC immediate OK\n");
}

void test_adc_opcode_zeropage(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 1,
      .x = 0,
      .y = 0};
  
  *mem[0] = 0x42;
  *mem[8] = 0x65; // adc zeropage
  *mem[9] = 0x0;

  run(mem, MEM_SIZE, &r);

  assert(r.a == 0x43);
  assert(r.pc == 10);
  assert(r.p.C == 0); // no carry

  hexdump(*mem, 16);

  printf("test: ADC zeropage OK\n");
}

void test_adc_opcode_zeropagex(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 1,
      .x = 0x02,
      .y = 0};
  
  *mem[0x04] = 0x42;
  *mem[8] = 0x75; // adc zeropagex
  *mem[9] = 0x02;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x43);
  assert(r.pc == 10);
  assert(r.p.C == 0); // no carry


  printf("test: ADC zeropagex OK\n");
}

void test_adc_opcode_abs(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 1,
      .x = 0x02,
      .y = 0};
  
  *mem[4] = 0x42;
  *mem[8] = 0x6d; // adc absolute
  *mem[9] = 0x04;
  *mem[10] = 0x00; //msb

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x43);
  assert(r.pc == 11);
  assert(r.p.C == 0); // no carry


  printf("test: ADC absolute OK\n");
}

void test_adc_opcode_absx(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 1,
      .x = 0x02,
      .y = 0};
  
  *mem[6] = 0x42;
  *mem[8] = 0x7d; // adc absolute x
  *mem[9] = 0x04;
  *mem[10] = 0x00; //msb

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x43);
  assert(r.pc == 11);
  assert(r.p.C == 0); // no carry


  printf("test: ADC absolute X OK\n");
}

void test_adc_opcode_absy(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 1,
      .x = 0,
      .y = 0x03};
  
  *mem[7] = 0x42;
  *mem[8] = 0x79; // adc absolute y
  *mem[9] = 0x04;
  *mem[10] = 0x00; //msb

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x43);
  assert(r.pc == 11);
  assert(r.p.C == 0); // no carry


  printf("test: ADC absolute Y OK\n");
}

void test_adc_opcode(t_mem *mem) {
  test_adc_opcode_immediate(mem);
  bzero(*mem, 16);
  test_adc_opcode_zeropage(mem);
  bzero(*mem, 16);
  test_adc_opcode_zeropagex(mem);
  bzero(*mem, 16);
  test_adc_opcode_abs(mem);
  bzero(*mem, 16);
  test_adc_opcode_absx(mem);
  bzero(*mem, 16);
  test_adc_opcode_absy(mem);
  bzero(*mem, 16);
}

void run_test(t_mem *mem) {
  test_adc_opcode(mem);
}