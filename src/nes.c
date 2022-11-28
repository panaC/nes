#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <assert.h>
#include "nes.h"
#include "cpu.h"
#include "parser.h"
#include "utils.h"
#include "clock.h"

#ifndef DEBUG_NES
#	define DEBUG_NES 1
#endif

#ifndef DEBUG_NES
#	define debug(...) 0;
#else
#	define debug_start() fprintf(stdout, "NES: ");
#	define debug_content(...) fprintf(stdout, __VA_ARGS__);
#	define debug_end() fprintf(stdout, "\n");
#	define debug(...) debug_start();debug_content(__VA_ARGS__);debug_end();
#endif

uint8_t __nes_ram[0x800] = {0};
uint8_t write_000_7ff(uint8_t value, uint32_t addr) {

  if (addr >= 0 && addr < 0x700) {
    __nes_ram[addr] = value;
  }

  return value;
}

uint8_t read_000_7ff(uint8_t value, uint32_t addr) {

  if (addr >= 0 && addr < 0x800) {
    return __nes_ram[addr];
  }
  return value;
}

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

uint8_t __ppu[9] = {0};
uint8_t write_ppu_register_log(uint8_t value, uint32_t addr) {

  if (addr >= 0x2000 && addr < 0x2008) {
    // debug_content("wPPU[0x%04x]=0x%2x | ", addr, value);
    __ppu[addr - 0x2000] = value;
  }
  return value;
}

int readingstatusregisteronce = 0;
uint8_t read_ppu_register_log(uint8_t value, uint32_t addr) {

  if (addr >= 0x2000 && addr < 0x2008) {
    value = __ppu[addr - 0x2000];
    // debug_content("rPPU[0x%04x]=0x%02x | ", addr, value);
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
    debug("WRITE APU or IO registers 0x%x=%d", addr, value);
  }
  return value;
}

int serialstateptr = -1; // 0-7 bit and -1 stop
uint8_t serialvalue = 0;
uint8_t serialbuf[0xffff] = {0};
uint32_t serialbufptr = 0;
FILE *write_ptr;
uint8_t write_apu_joy1_serial(uint8_t value, uint32_t addr) {

  if (addr == 0x4016) {

    if (serialstateptr >= 0 && serialstateptr < 8) {
      // read uart data

      serialvalue |= value << serialstateptr++;
    } else if (serialstateptr == 8) {
      assert(value == 1); // stop
      serialstateptr = -1; // stop
      serialbuf[serialbufptr] = serialvalue;

      // TODO: not very clean to do this
      fwrite(serialbuf + serialbufptr, 1, 1, write_ptr);
      fflush(write_ptr);
      serialbufptr++;

    }
    else if (value == 0)
    {
      serialstateptr = 0; // start
      serialvalue = 0;
    }
  }
  return value;
}
/***
 *
 * SERIAL:
01 01 01 01 01 01 01 01  01 01 00 00 01 00 01 00  |  ................
00 00 00 01 00 00 00 01  00 00 01 00 00 01 00 00  |  ................
01 00 00 01 01 00 00 01  00 00 00 00 00 01 01 00  |  ................
00 01 00 00 00 00 00 01  01 00 00 01 00 00 01 00  |  ................
00 01 01 00 00 01 00 00  00 00 00 00 01 00 00 01  |  ................
00 01 00 01 01 00 01 01  00 01 00 01 00 01 00 01  |  ................
01 01 00 01 00                                    |  .....
*/

uint8_t *_prgrom = NULL;
uint8_t _prgrom_size = 0; // in x * 16Kb
uint8_t read_cartridge(uint8_t value, uint32_t addr) {

  if (addr >= 0x8000 && addr <= 0xffff) {
    uint32_t ref = addr - 0x8000;
    // return _prgrom[ref % (16384 * _prgrom_size)];
    return _prgrom[ref];
  } else if (addr >= 0x4020) {
    debug("READ Unknown Cartridge Space 0x%x=%d", addr, value);
  }
  return value;
}

uint8_t write_catridge(uint8_t value, uint32_t addr) {
  if (addr >= 0x8000 && addr <= 0xffff) {
    debug("WRITE to Cartridge Space WHY ? 0x%x=%d", addr, value);
    // assert(0); // error read only
  } else if (addr >= 0x4020) {
    debug("WRITE Unknown Cartridge Space 0x%x=%d", addr, value);
    // assert(0); // Read only ?
  }
  return value;
}

uint8_t assert_memory(uint8_t value, uint32_t addr) {

  assert(addr < MEM_SIZE);
  return value;
}

void mapper0(struct s_ines_parsed ines) {

  _prgrom = ines.prgrom;
  _prgrom_size = ines.ines->size_prgrom;

  debug("PRGROM start at %d with size of %d", _prgrom - ines.ptr, _prgrom_size * 0x4000);

  // hexdump(_prgrom, _prgrom_size * 0x4000);

  // FILE *write_ptr;

  // write_ptr = fopen("bump.bin","wb");  // w for write, b for binary

  // fwrite(_prgrom, _prgrom_size * 0x4000, 1, write_ptr);
}

uint8_t nes_readbus(uint32_t addr) {

  uint8_t value = 0;
  
  value = assert_memory(value, addr);
  value = read_000_7ff(value, addr);
  value = read_mirror_000_7ff(value, addr);
  value = read_ppu_register_log(value, addr);
  value = read_cartridge(value, addr);

  debug_content("r[0x%04x]=%02x | ", addr, value);
  return value;
}

void nes_writebus(uint32_t addr, uint8_t value) {

  value = assert_memory(value, addr);
  value = write_000_7ff(value, addr);
  value = write_mirror_000_7ff(value, addr);
  value = write_ppu_register_log(value, addr);
  value = write_mirror_ppu(value, addr);
  value = write_catridge(value, addr);
  value = write_apu_joy1_serial(value, addr);

  debug_content("w[0x%04x]=%02x | ", addr, value);
}

static void nes_init(struct s_ines_parsed ines) {

  uint32_t size = 0x800; // 2kb internal memory
  uint8_t *rawmem = (uint8_t *)malloc(size);
  bzero(rawmem, size);

  // TODO
  write_ptr = fopen("serial.txt", "wb"); // w for write, b for binary

  // PPU init
  // https://www.nesdev.org/wiki/PPU_registers#PPUSTATUS
  // 0x2002 bit 7 should equal one when the ppu is initialize
  // https://github.com/christopherpow/nes-test-roms/blob/95d8f621ae55cee0d09b91519a8989ae0e64753b/cpu_dummy_reads/source/common/shell.inc#L157
  __ppu[2] = 0x80;

  cpu_readbus = &nes_readbus;
  cpu_writebus = &nes_writebus;
  mapper0(ines);

}

// main entry
int nes(struct s_ines_parsed ines)
{
  nes_init(ines);
  cpu_init();

  int cycles = 0;
  struct timespec t1 = {0};
  struct timespec t2 = {0};
  uint64_t time_elasped = 0;
  uint64_t freq_delay = 1.0e9 / CPU_FREQ;

  struct timespec sleep = {
    .tv_sec = 0,
    .tv_nsec = freq_delay,
  };

  for (;;) {
    debug_start();

    clock_gettime(CLOCK_REALTIME, &t1);

    cycles = cpu_exec(NULL);
    if (cycles == -1)
      break;

    // clock_gettime(CLOCK_MONOTONIC, &t2);
    clock_gettime(CLOCK_REALTIME, &t2);

    time_elasped = t2.tv_nsec - t1.tv_nsec < 0 ? (1.0e9 + (t2.tv_nsec - t1.tv_nsec)) : (t2.tv_nsec - t1.tv_nsec);
    debug_content("TIME=%ld ", time_elasped);
    assert(freq_delay - time_elasped > 0);
    sleep.tv_nsec = freq_delay - time_elasped;
    nanosleep(&sleep, NULL);

    debug_end();
  }

  debug("QUIT with %d", cycles);
  return cycles;
}

/**
 * clock rate
 * 
 * https://www.nesdev.org/wiki/Cycle_reference_chart#Clock_rates
 * 
 * For NTSC (2C02)
 * 
 * Master clock speed:
 * 21.477272 MHz ± 40 Hz
 * 236.25 MHz ÷ 11 by definition
 * 
 * cpu clock:
 * 21.47~ MHz ÷ 12 = 1.789773 MHz
 * 
 * ppu clock:
 * 21.477272 MHz ÷ 4
 * = 5.369318 MHz
 * 
 * ppu tick 1/f = 186.24ns
 * cpu tick 1/f = 558.73ns
 * 
 * 
 * ppu tick is not important because we use the sdl lib to rendering.
 * The master clock remains the cpu one at 1.789Mhz.
 * 
 * To render a frame it needs 89341.5 ÷ 3 = 29780.5 cpu cycles
 * 
 */

/**
 *
 * Threading
 *
 * in nes system there are 2 logic chip the cpu and the gpu (ppu)
 * the cpu triggers the gpu 
 *
 * cpu threading is important to halt the cpu loop in debugging
 * ppu threading is necessary to catch event from cpu bus 
 * The PPU exposes eight memory-mapped registers to the CPU $2000 to $2007
 *
 *  -- UPDATE -- 
 * no threading in first phase
 *
 * */

