
#include <iostream>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "cpu.h"

int main(int argc, char **argv) {

  struct s_registers registers = {};

  union u_pc pc;

  pc.value = 0x1234;

  registers.pc = pc;

  std::cout << "nes" << std::endl;
  std::cout << "pc: " << std::hex << pc.value << std::endl;

  pc.pcl = 0x00;
  pc.pch = 0xff;

  std::cout << "pc: " << std::hex << pc.value << std::endl;

  union u_p p;

  p.value = 0x00;
  p.unused = 1; // this flag is always 1
  p.N = 1;

  registers.p = p;

  std::cout << "p: " << std::bitset<8>(p.value) << std::endl;

  std::cout << "registers : " << std::endl;
  std::cout << "P=" << std::hex << p.value << std::endl;
  std::cout << "PC=" << std::hex << registers.pc.value << std::endl;
  std::cout << std::endl;

 
  return 0;
}
