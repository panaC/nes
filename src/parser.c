#include "debug.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>  
#include <sys/mman.h>
#include "parser.h"
#include "log.h"

#define debug(...) log_x(LOG_PARSER, __VA_ARGS__)

void parse(char* filepath) {

  int fd = open(filepath, O_RDONLY);
  if (fd < 0) {
    debug("Error to open %s error=%s", filepath, strerror(errno));
    return ;
  }

  struct stat st;
  fstat(fd, &st);

  uint8_t *ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (ptr == NULL) {
    return ;
  }
  debug("ines file size %lld", st.st_size);
  if (st.st_size < 16 || !(st.st_size > 16 && !strncmp((char*)ptr, "NES\x1a", 4))) {
    fprintf(stderr, "Not an INES format\n");
    return ;
  }

  struct s_ines *ines = (struct s_ines*) ptr;

  debug("pgrrom size %d", ines->size_pgrrom);
  debug("chrrom size %d", ines->size_chrrom);
  debug("prgrram size %d", ines->size_pgrram);

  if (ines->flag6.mirroring) {
    debug("mirroring vertical (horizontal arrangement) (CIRAM A10 = PPU A10)");
  } else {
    debug("mirroring horizontal (vertical arrangement) (CIRAM A10 = PPU A11)");
  }

  if (ines->flag6.isBatteryPacked) {
    debug("battery Packed");
  }

  if (ines->flag6.isTrainer) {
    debug("is Trainer");
  }
  
  if (ines->flag6.ignoreMirroring) {
    debug("Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM");
  }

  if (ines->flag7.isVSUnisystem) {
    debug("isVSUnisystem");
  }

  if (ines->flag7.isPlayChoice10) {
    debug("isPlayChoice10");
  }

  if (ines->flag7.isNes2Format) {
    debug("nes2.0");
  }

  debug("mapper=%x", ines->flag6.lowerNibbleMapperNumber && (ines->flag7.upperNibbleMapperNumber << 4));

  if (ines->tvsystem) {
    debug("TV PAL");
  } else {
    debug("TV NTSC");
  }


  close(fd);
}

