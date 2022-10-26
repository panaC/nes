#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// http://fms.komkon.org/EMUL8/NES.html
// https://www.nesdev.org/wiki/INES

/**
 * An iNES file consists of the following sections, in order:

   - Header (16 bytes)
   - Trainer, if present (0 or 512 bytes)
   - PRG ROM data (16384 * x bytes)
   - CHR ROM data, if present (8192 * y bytes)
   - PlayChoice INST-ROM, if present (0 or 8192 bytes)
   - PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut) (this is often missing, see PC10 ROM-Images for details)
   - Some ROM-Images additionally contain a 128-byte (or sometimes 127-byte) title at the end of the file.

 */
struct s_ines {

  int8_t magic[4];
  uint8_t size_prgrom; // Size of PRG ROM in 16 KB units
  uint8_t size_chrrom; // Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
  union flag6 {
    uint8_t value;

    struct
    {
      unsigned mirroring : 1; // Mirroring: 0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11) 1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
      unsigned isBatteryPacked : 1; // 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
      unsigned isTrainer : 1; // 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
      unsigned ignoreMirroring : 1; // 1: Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM
      unsigned lowerNibbleMapperNumber : 4; 
    };
  } flag6;
  union flag7
  {
    uint8_t value;

    struct
    {
      unsigned isVSUnisystem : 1;
      unsigned isPlayChoice10 : 1; // PlayChoice-10 (8KB of Hint Screen data stored after CHR data)
      unsigned isNes2Format : 2;      // If equal to 2, flags 8-15 are in NES 2.0 format
      unsigned upperNibbleMapperNumber : 4; 
    };
  } flag7;
  uint8_t size_pgrram; // PRG-RAM size (rarely used extension)
  uint8_t tvsystem; // TV system (0: NTSC; 1: PAL)
  // Though in the official specification, very few emulators honor this bit as virtually no ROM images in circulation make use of it.
  union flag10
  {
    uint8_t value;

    struct
    {
      unsigned tvsystem : 2; // TV system (0: NTSC; 2: PAL; 1/3: dual compatible)
      unsigned unused : 2; // unused
      unsigned isPrgram : 1; // PRG RAM ($6000-$7FFF) (0: present; 1: not present)
      unsigned isBusConflict : 1; // 0: Board has no bus conflicts; 1: Board has bus conflicts https://www.nesdev.org/wiki/Bus_conflict
      unsigned unused_ : 2; // unused
    };
  } flag10;
  // This byte is not part of the official specification, and relatively few emulators honor it.
  uint8_t flag11;
  uint8_t flag12;
  uint8_t flag13;
  uint8_t flag14;
  uint8_t flag15;
};
// 16 bytes padding

// Older versions of the iNES emulator ignored bytes 7-15, and several ROM management tools wrote messages in there. Commonly, these will be filled with "DiskDude!", which results in 64 being added to the mapper number.
// TODO A general rule of thumb: if the last 4 bytes are not all zero, and the header is not marked for NES 2.0 format, an emulator should either mask off the upper 4 bits of the mapper number or simply refuse to load the ROM.

struct s_ines_parsed {
  uint8_t *ptr;
  uint64_t size;
  struct s_ines *ines;
  uint8_t *trainer;
  uint8_t *prgrom;
  uint8_t *chrrom;
};

struct s_ines_parsed* parse(char* filepath);



#endif