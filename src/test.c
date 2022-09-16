#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "test.h"
#include "utils.h"
#include "bus.h"

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

void test_and_opcode_immediate(t_mem *mem)
{

  t_registers r = {
      .pc = 0,
      .sp = 0,
      .p = 0,
      .a = 0x0f,
      .x = 0,
      .y = 0};
  
  *mem[0] = 0x29; // adc immediate
  *mem[1] = 0xf0;

  run(mem, MEM_SIZE, &r);

  assert(r.a == 0x00);
  assert(r.pc == 2);

  hexdump(*mem, 16);

  printf("test: AND immediate OK\n");
}

void test_and_opcode_zeropage(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0x0f,
      .x = 0,
      .y = 0};
  
  *mem[0] = 0x0c;
  *mem[8] = 0x25;
  *mem[9] = 0x0;

  run(mem, MEM_SIZE, &r);

  assert(r.a == 0x0c);
  assert(r.pc == 10);
  assert(r.p.C == 0); // no carry

  hexdump(*mem, 16);

  printf("test: AND zeropage OK\n");
}

void test_and_opcode_zeropagex(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0xff,
      .x = 0x02,
      .y = 0};
  
  *mem[0x04] = 0x42;
  *mem[8] = 0x35; 
  *mem[9] = 0x02;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x42);
  assert(r.pc == 10);
  assert(r.p.C == 0); // no carry


  printf("test: AND zeropagex OK\n");
}

void test_and_opcode_absolute(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0xff,
      .x = 0x02,
      .y = 0};
  
  *mem[0x04] = 0x42;
  *mem[8] = 0x2d;
  *mem[9] = 0x04;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x42);
  assert(r.pc == 11);
  assert(r.p.C == 0); // no carry


  printf("test: AND absolute OK\n");
}

void test_and_opcode_absolutex(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0xff,
      .x = 0x02,
      .y = 0};
  
  *mem[0x06] = 0x42;
  *mem[8] = 0x3d;
  *mem[9] = 0x04;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x42);
  assert(r.pc == 11);
  assert(r.p.C == 0); // no carry


  printf("test: AND absolutex OK\n");
}

void test_and_opcode_absolutey(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0xff,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x05] = 0x42;
  *mem[8] = 0x39;
  *mem[9] = 0x04;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x42);
  assert(r.pc == 11);
  assert(r.p.C == 0); // no carry


  printf("test: AND absolutey OK\n");
}

void test_and_opcode(t_mem *mem) {
  test_and_opcode_immediate(mem);
  bzero(*mem, 16);
  test_and_opcode_zeropage(mem);
  bzero(*mem, 16);
  test_and_opcode_zeropagex(mem);
  bzero(*mem, 16);
  test_and_opcode_absolute(mem);
  bzero(*mem, 16);
  test_and_opcode_absolutex(mem);
  bzero(*mem, 16);
  test_and_opcode_absolutey(mem);
  bzero(*mem, 16);
}

void test_asl_opcode_immediate(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0x0f,
      .x = 0x02,
      .y = 0x01};
  
  *mem[8] = 0x0a;
  *mem[9] = 0x00;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.a == 0x1e);
  assert(r.pc == 9);
  assert(r.p.C == 0); // no carry


  printf("test: ASL immediate OK\n");
}

void test_asl_opcode_zeropage(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x03] = 0x0f;
  *mem[8] = 0x06;
  *mem[9] = 0x03;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(*mem[0x03] == 0x1e);
  assert(r.pc == 10);
  assert(r.p.C == 0); // no carry


  printf("test: ASL zeropage OK\n");
}

void test_asl_opcode_zeropagex(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x03] = 0x0f;
  *mem[8] = 0x16;
  *mem[9] = 0x01;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(*mem[0x03] == 0x1e);
  assert(r.pc == 10);
  assert(r.p.C == 0); // no carry


  printf("test: ASL zeropagex OK\n");
}

void test_asl_opcode_absolute(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x01] = 0x0f;
  *mem[8] = 0x0e;
  *mem[9] = 0x01;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(*mem[0x01] == 0x1e);
  assert(r.pc == 11);
  assert(r.p.C == 0); // no carry


  printf("test: ASL absolute OK\n");
}

void test_asl_opcode_absolutex(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x03] = 0x0f;
  *mem[8] = 0x1e;
  *mem[9] = 0x01;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(*mem[0x03] == 0x1e);
  assert(r.pc == 11);
  assert(r.p.C == 0); // no carry


  printf("test: ASL absolutex OK\n");
}

void test_asl_opcode(t_mem *mem) {
  test_asl_opcode_immediate(mem);
  bzero(*mem, 16);
  test_asl_opcode_zeropage(mem);
  bzero(*mem, 16);
  test_asl_opcode_zeropagex(mem);
  bzero(*mem, 16);
  test_asl_opcode_absolute(mem);
  bzero(*mem, 16);
  test_asl_opcode_absolutex(mem);
  bzero(*mem, 16);
}

void test_bcc_opcode_relative(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x03] = 0x0f;
  *mem[8] = 0x90;
  *mem[9] = 0x0f;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.pc == 8 + 0x0f); // WARNING : The jump equal 10 + 0x0f or 8 + 0x0f

  printf("test: BCC relative OK\n");
}

void test_bcs_opcode_relative(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = {.C = 1},
      .a = 0,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x03] = 0x0f;
  *mem[8] = 0xb0;
  *mem[9] = 0x0f;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.pc == 8 + 0x0f); // WARNING : The jump equal 10 + 0x0f or 8 + 0x0f

  printf("test: BCS relative OK\n");
}

void test_beq_opcode_relative(t_mem *mem)
{

  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = {.Z = 1},
      .a = 0,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x03] = 0x0f;
  *mem[8] = 0xf0;
  *mem[9] = 0x0f;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.pc == 8 + 0x0f); // WARNING : The jump equal 10 + 0x0f or 8 + 0x0f

  printf("test: BEQ relative OK\n");
}

void test_bit_opcode_zeropage(t_mem *mem) {
  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0xff,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x03] = 0xf0;
  *mem[8] = 0x24;
  *mem[9] = 0x03;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.p.V == 1);
  assert(r.p.N == 1);
  assert(r.pc == 10);

  printf("test: BIT zeropage OK\n");
} 

void test_bit_opcode_absolute(t_mem *mem) {
  t_registers r = {
      .pc = 8,
      .sp = 0,
      .p = 0,
      .a = 0xff,
      .x = 0x02,
      .y = 0x01};
  
  *mem[0x03] = 0xf0;
  *mem[8] = 0x2c;
  *mem[9] = 0x03;

  run(mem, MEM_SIZE, &r);

  hexdump(*mem, 16);

  assert(r.p.V == 1);
  assert(r.p.N == 1);
  assert(r.pc == 11);

  printf("test: BIT absolute OK\n");
} 

void test_bit_opcode(t_mem *mem) {
  test_bit_opcode_zeropage(mem);
  bzero(*mem, 16);
  test_bit_opcode_absolute(mem);
  bzero(*mem, 16);
}

void run_test(t_mem *mem) {
  test_adc_opcode(mem);
  test_and_opcode(mem);
  test_asl_opcode(mem);
  test_bcc_opcode_relative(mem);
  bzero(*mem, 16);
  test_bcs_opcode_relative(mem);
  bzero(*mem, 16);
  test_beq_opcode_relative(mem);
  bzero(*mem, 16);
  test_bit_opcode(mem);
}