#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <strings.h>
#include <assert.h>
#include "nes.h"
#include "cpu.h"
#include "parser.h"
#include "utils.h"

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
uint8_t __ppu[9] = {0};
uint8_t __apu[0x20] = {0};

uint8_t *_prgrom = NULL;
uint8_t _prgrom_size = 0; // in x * 16Kb
int _prgrom_size_human_readable = 0;

void mapper0(struct s_ines_parsed ines) {

  _prgrom = ines.prgrom;
  _prgrom_size = ines.ines->size_prgrom;
  _prgrom_size_human_readable = _prgrom_size * 0x4000;

  debug("PRGROM start at %d with size of %d", _prgrom - ines.ptr, _prgrom_size_human_readable);

}

static uint8_t write_apu(uint32_t addr, uint8_t value) {

  if (addr >= 0x4000 && addr < 0x4020) {
    __apu[addr - 0x4000] = value;
  }
  return value;
}

static uint8_t read_apu(uint32_t addr, uint8_t value) {

  if (addr >= 0x4000 && addr < 0x4020) {
    return __apu[addr - 0x4000];
  }
  return value;
}

// TODO: Debug only for romtest
int serialstateptr = -1; // 0-7 bit and -1 stop
uint8_t serialvalue = 0;
uint8_t serialbuf[0xffff] = {0};
uint32_t serialbufptr = 0;
FILE *write_ptr;
static uint8_t write_apu_joy1_serial(uint32_t addr, uint8_t value) {

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

static uint8_t read_cartridge(uint32_t addr, uint8_t value) {

  uint32_t addr_offset = addr - 0x8000;

  if (addr >= 0x8000 && addr <= 0xffff && addr_offset < _prgrom_size_human_readable) {
    return _prgrom[addr_offset];
  } else if (addr >= 0x4020) {
    debug_content("READ Unknown Cartridge Space with a size of %x ", _prgrom_size_human_readable);
  }
  return value;
}

static uint8_t write_catridge(uint32_t addr, uint8_t value) {
  if (addr >= 0x8000 && addr <= 0xffff) {
    debug_content("WRITE to Cartridge Space WHY ?");
    // assert(0); // error read only
  } else if (addr >= 0x4020) {
    debug_content("WRITE Unknown Cartridge Space");
    // assert(0); // Read only ?
  }
  return value;
}

static uint8_t read_000_7ff(uint32_t addr, uint8_t value) {

  uint32_t addr_array[] = {0, 0x800, 0x1000, 0x1800};
  uint32_t addr_offset = 0;
  for (int i = 0; i < sizeof(addr_array); i++) {
    addr_offset = addr - addr_array[i];
    if (addr_offset >= 0 && addr_offset < 0x800) {
      return __nes_ram[addr_offset];
    }
  }
  return value;
}

static uint8_t write_000_7ff(uint32_t addr, uint8_t value) {

  if (addr >= 0 && addr < 0x2000) {
    __nes_ram[addr % 0x800] = value;
  }
  return value;
}

static uint8_t read_ppu(uint32_t addr, uint8_t value) {

  uint32_t addr_offset = 0;
  if (addr >= 0x2000 && addr < 0x4000) {
    addr_offset = (addr - 0x2000) % 8;
    value = __ppu[0x2000 + addr_offset];
  }
  return value;
}

static uint8_t write_ppu(uint32_t addr, uint8_t value) {
  uint32_t addr_offset = 0;
  if (addr >= 0x2000 && addr < 0x4000) {
    addr_offset = (addr - 0x2000) % 8;
    __ppu[0x2000 + addr_offset] = value;
  }
  return value;
}

uint8_t nes_readbus(uint32_t addr) {

  uint8_t value = 0;

  assert(addr < MEM_SIZE);

  value = read_000_7ff(addr, value);
  value = read_ppu(addr, value);
  value = read_apu(addr, value);
  value = read_cartridge(addr, value);

  debug_content("r[0x%04x]=%02x | ", addr, value);
  return value;
}

void nes_writebus(uint32_t addr, uint8_t value) {
  assert(addr < MEM_SIZE);

  value = write_000_7ff(addr, value);
  value = write_ppu(addr, value);
  value = write_apu_joy1_serial(addr, value);
  value = write_apu(addr, value);
  value = write_catridge(addr, value);

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

