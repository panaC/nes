#include <string>
#include "Cpu.h"
#include "../src/cpu.h"
#include <stdio.h> 

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Cpu );


void 
Cpu::setUp()
{
}


void 
Cpu::tearDown()
{
}


void 
Cpu::bitwise()
{
  // CPPUNIT_FAIL( "not implemented" );

 struct s_registers registers = {};

  union u_pc pc;

  pc.value = 0x1234;

  registers.pc = pc;

  pc.pcl = 0xff;
  pc.pch = 0x00;

  registers.pc.pcl = 0x00;
  registers.pc.pch = 0xff;

  CPPUNIT_ASSERT(registers.pc.value == 0xff00);
  CPPUNIT_ASSERT(pc.value == 0x00ff);

  registers.p.value = 0x00;
  registers.p.unused = 1; // this flag is always 1
  registers.p.N = 1;

  CPPUNIT_ASSERT(registers.p.value == 0b10100000);
}
