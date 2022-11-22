#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>

struct s_clock_thread_arg {
  pthread_cond_t *condition_cond;  
  pthread_mutex_t *condition_mutex;
};
void *clock_thread(void *arg);

#endif