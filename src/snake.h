#ifndef SNAKE_H
#define SNAKE_H

#include "cpu.h"

#define SNAKE_HEIGHT 640
#define SNAKE_WIDTH 640
#define SNAKE_PITCH 20

typedef enum {
  NO_EVENT = 0,
  QUIT_EVENT,
  MOVE_UP,
  MOVE_DOWN,
  MOVE_LEFT,
  MOVE_RIGHT,
  PAUSE
} t_snake_action;

void snake_init();
void snake();

extern char __pause;

#endif