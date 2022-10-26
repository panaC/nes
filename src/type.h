#ifndef TYPE_H
#define TYPE_H

union u16 {

  uint16_t value;

  struct {

    uint8_t lsb;
    uint8_t msb;
  };
};

#endif