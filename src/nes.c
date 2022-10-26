#include <stdlib.h>
#include <strings.h>
#include <assert.h>
#include "nes.h"
#include "log.h"
#include "cpu.h"
#include "parser.h"

#define debug(...) log_x(LOG_BUS, __VA_ARGS__)

uint8_t write_mirror_000_7ff(uint8_t value, uint32_t addr) {

  if (addr >= 0 && addr < 0x700) {

    cpu_writebus(addr + 0x800, value);
    cpu_writebus(addr + 0x1000, value);
    cpu_writebus(addr + 0x1800, value);
  }

  return value;
}

uint8_t read_mirror_000_7ff(uint8_t value, uint32_t addr) {

  uint32_t t[] = {0x800, 0x1000, 0x1800};
  for (int i = 0; i < 3; i++) {
    uint32_t addr2 = addr - t[i];
    if (addr2 >= 0 && addr2 < 0x800) {
      return cpu_readbus(addr2);
    }
  }
  return value;
}

uint8_t write_ppu_register_log(uint8_t value, uint32_t addr) {

  if (addr >= 0x2000 && addr < 0x2008) {
    debug("PPU REGISTER 0x%x=%d", addr, value);
  }
  return value;
}

uint8_t read_mirror_ppu(uint8_t value, uint32_t addr) {

  if (addr >= 0x2008 && addr < 0x4000) {

    uint32_t ref = addr - 0x2008; // offset
    return cpu_readbus(0x2000 + (ref % 8));
  }
  return value;
}

uint8_t write_mirror_ppu(uint8_t value, uint32_t addr) {

  if (addr >= 0x2008 && addr < 0x4000) {

    uint32_t ref = addr - 0x2008; // offset
    cpu_writebus(0x2000 + (ref % 8), value);
  }
  return value;
}

uint8_t write_apu_io_log(uint8_t value, uint32_t addr) {

  if (addr >= 0x4000 && addr < 0x4020) {
    debug("APU or IO registers 0x%x=%d", addr, value);
  }
  return value;
}

uint8_t *_prgrom = NULL;
uint8_t _prgrom_size = 0; // in x * 16Kb
uint8_t read_prgrom(uint8_t value, uint32_t addr) {
  if (addr >= 0x8000 && addr <= 0xffff) {
    uint32_t ref = addr - 0x8000;
    return _prgrom[ref % (16384 * _prgrom_size)];
  }
  return value;
}

uint8_t write_prgrom(uint8_t value, uint32_t addr) {
  if (addr >= 0x8000 && addr <= 0xffff) {
    assert(0); // error read only
  }
  return value;
}

void mapper0(struct s_ines_parsed ines) {

  _prgrom = ines.prgrom;
  _prgrom_size = ines.ines->size_prgrom;
}

static void nes_init(struct s_ines_parsed ines) {

  uint8_t *rawmem = (uint8_t *)malloc(MEM_SIZE);
  bzero(rawmem, MEM_SIZE);
  for (int i = 0; i < MEM_SIZE; i++) {
    __cpu_memory[i] = rawmem + i;
  }

  cpu_write_on(&write_mirror_000_7ff);
  cpu_read_on(&read_mirror_000_7ff);
  cpu_write_on(&write_ppu_register_log);
  cpu_read_on(&read_mirror_ppu);
  cpu_write_on(&write_mirror_ppu);
  cpu_read_on(&read_prgrom);
  cpu_read_on(&write_prgrom);

  mapper0(ines);

}

// main entry
void nes(struct s_ines_parsed ines) {

  nes_init(ines);

}