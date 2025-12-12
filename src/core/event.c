/**
  * @file   event.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   30-September-2025
  */

#include "event.h"
#include "alloc.h"
#if (EV_USE_THREADS)
#include "port_semph.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 


struct event_s {
  ev_type_t type;
  bool      active;
#if (EV_USE_THREADS)
  semph_t   sem;
#endif
};
STATIC_DECLARE(EVENT, struct event_s);


/**
  * @brief Constructor
  */
ev_t ev_init(void)
{
  ALLOC(EVENT, struct event_s, self, return NULL);
  self->type = EV_NONE;
  self->active = false;
#if (EV_USE_THREADS)
  self->sem = semph_new();
  if (!self->sem) {
    FREE(EVENT, self);
    return NULL;
  }
#endif
  
  return self;
}

/**
  * @brief Destructor
  */
void ev_del(ev_t self)
{
  assert(self);
#if (EV_USE_THREADS)
  semph_del(self->sem);
#endif
  FREE(EVENT, self);
}

/**
  * @brief Semaphore give
  */
void ev_post(ev_t self, ev_type_t type)
{
  assert(self);
  self->type = type;
  self->active = true;
#if (EV_USE_THREADS)
  semph_post(self->sem);
#endif
}

/**
  * @brief Try take the semaphore else block
  */
bool ev_get(ev_t self, ev_type_t *ptype)
{
  assert(self);
#if (EV_USE_THREADS)
  semph_wait(self->sem);
  *ptype = self->type;
  self->active = false;
  return true;
#else
  if (self->active) {
    *ptype = self->type;
    self->active = false;
    return true;
  }
  return false;
#endif
}
