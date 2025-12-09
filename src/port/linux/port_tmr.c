/**
  * @file   port_tmr.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   05-October-2025
  */

#ifndef __unix__
#error "Should only be compiled under a unix system"
#endif

#include "port_tmr.h"
#include "port_alloc.h"
#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tmr_s {
  bool            enabled;
  u32_t           timeout;
  struct timeval  time_last;
  tmr_tick_t      fn;
  tmr_pld_t       pld;
};

PORT_STATIC_DECLARE(TMR, struct tmr_s);

// ============================= Публичные функции =============================

/**
  * @brief  Constructor
  * @param  timeout - Responce timeout, ms
  * @param  fn - 
  * @param  pld - 
  * @retval Pointer to the object itself
  */
tmr_t tmr__init(u32_t timeout, tmr_tick_t fn, tmr_pld_t pld)
{
  PORT_ALLOC(TMR, struct tmr_s, self, return NULL);
  assert( fn && pld && (timeout > 0) );

  self->enabled = false;
  self->timeout = timeout;
  self->fn = fn;
  self->pld = pld;
  
  printf("[tmr__init]\n");
  return self;
}

/**
  * @brief Destructor
  * @param self - Pointer to the object itself
  */
void tmr__del(tmr_t self)
{
  assert(self);
  
  printf("[tmr__del]\n");
  if (self->enabled) {
    self->enabled = false;
  }
  tmr__dis(self);
  PORT_FREE(TMR, self);
}

/**
  * @brief ?
  * @param self - Pointer to the object itself
  */
void tmr__poll(tmr_t self)
{
  u32_t          delta_ms;
  struct timeval time_now;
  //struct timeval diff;
  long sec_diff, usec_diff;
  assert(self);
  if (!self->enabled) return;
  
  if (gettimeofday(&time_now, NULL)) {
    perror("Trying to get the time is failed");
    return;
  }
  
  // вычисление разницы
  // if (time_now.tv_usec < self->time_last.tv_usec) {
      // time_now.tv_sec--;
      // time_now.tv_usec += 1000000;
  // }
  // diff.tv_sec = time_now.tv_sec - self->time_last.tv_sec;
  // diff.tv_usec = time_now.tv_usec - self->time_last.tv_usec;
  // printf("[tmr__poll] diff is %10ld.%06ld\r\n", diff.tv_sec, diff.tv_usec);
  
  sec_diff = time_now.tv_sec - self->time_last.tv_sec;
  usec_diff = time_now.tv_usec - self->time_last.tv_usec;
  if (usec_diff < 0) {
    sec_diff--;
    usec_diff += 1000000;
  }
  delta_ms = sec_diff * 1000L + usec_diff / 1000L;
  if( delta_ms > self->timeout ) {
    //self->enabled = false; TODO delete
    if (self->fn) self->fn(self->pld);
  }
  
  // printf("[tmr__poll] Epochtime now is  %08ld\n", time_now.tv_sec);
  // printf("[tmr__poll] Epochtime last is %08ld\n", self->time_last.tv_sec);
  // printf("[tmr__poll] delta_ms is       %08d (ms)\r\n", delta_ms);
}

/**
  * @brief ?
  */
s32_t tmr__ena(tmr_t self)
{
  assert(self);
  if (gettimeofday( &self->time_last, NULL )) return -1;
  self->enabled = true;
  return 0;
}

/**
  * @brief ?
  */
void tmr__dis(tmr_t self)
{
  assert(self);
  self->enabled = false;
}
