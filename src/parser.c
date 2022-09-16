#include "debug.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>  
#include <sys/mman.h>
#include "parser.h"

void parse(char* filepath) {

  int fd = open(filepath, O_RDONLY);
  if (fd < 0) {
    VB0(printf("Error to open %s error=%s", filepath, strerror(errno)));
    return ;
  }

  struct stat st;
  fstat(fd, &st);

  uint8_t *ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (ptr == NULL) {
    return ;
  }
  VB4(printf("ines file size %lld", st.st_size));
  if (st.st_size < 16 || !(st.st_size > 16 && !strncmp((char*)ptr, "NES\x1a", 4))) {
    fprintf(stderr, "Not an INES format\n");
    return ;
  }

  struct s_ines *ines = (struct s_ines*) ptr;

  VB0(printf("pgrrom size %d", ines->size_pgrrom));
  VB0(printf("chrrom size %d", ines->size_chrrom));
  VB0(printf("prgrram size %d", ines->size_pgrram));

  if (ines->flag6.mirroring) {
    VB0(printf("mirroring vertical (horizontal arrangement) (CIRAM A10 = PPU A10)"));
  } else {
    VB0(printf("mirroring horizontal (vertical arrangement) (CIRAM A10 = PPU A11)"));
  }

  if (ines->flag6.isBatteryPacked) {
    VB0(printf("battery Packed"));
  }

  if (ines->flag6.isTrainer) {
    VB0(printf("is Trainer"));
  }
  
  if (ines->flag6.ignoreMirroring) {
    VB0(printf("Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM"));
  }

  if (ines->flag7.isVSUnisystem) {
    VB0(printf("isVSUnisystem"));
  }

  if (ines->flag7.isPlayChoice10) {
    VB0(printf("isPlayChoice10"));
  }

  if (ines->flag7.isNes2Format) {
    VB0(printf("nes2.0"));
  }

  VB0(printf("mapper=%x", ines->flag6.lowerNibbleMapperNumber && (ines->flag7.upperNibbleMapperNumber << 4)));

  if (ines->tvsystem) {
    VB0(printf("TV PAL"));
  } else {
    VB0(printf("TV NTSC"));
  }


  close(fd);
}

