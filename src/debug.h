#ifndef DEBUG_H
#define DEBUG_H

#include "cpu.h"

int global_verbose_level;

#define VB4(X) if (global_verbose_level >= 4) printf("    "); X; puts("");
#define VB3(X) if (global_verbose_level >= 3) printf("   "); X; puts("");
#define VB2(X) if (global_verbose_level >= 2) printf("  "); X; puts("");
#define VB1(X) if (global_verbose_level >= 1) printf(" "); X; puts("");
#define VB0(X) X; puts("");

void print_register(t_registers *reg);

#endif