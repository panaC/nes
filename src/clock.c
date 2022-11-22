#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "clock.h"

void* clock_thread(void *arg) {

  struct s_clock_thread_arg *clock_arg = arg;

  const uint64_t t = (1000 * 1000 * 1000) / CPU_FREQ; // tick every 1ns // limit to 1Ghz
  struct timespec tspec = {.tv_sec = 0, .tv_nsec = t};

  int result = 0;
  while (1) {
    result = nanosleep(&tspec, NULL);
    if (result == 0) {
      pthread_mutex_lock(clock_arg->condition_mutex);
      pthread_cond_signal(clock_arg->condition_cond);
      pthread_mutex_unlock(clock_arg->condition_mutex);
    } else {
      perror("CLOCK MISSING NANOSLEEP ERROR");
    }
  }

  return NULL;
}
