/**
  * @file   event.c
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   30-September-2025
  */

//#define EV_USE_THREADS                (PORT_USE_THREADS)
#define EV_USE_THREADS                  (0)

#include "ser2mms_conf.h"
#include "event.h"
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

#if (S2M_USE_STATIC)
static struct event_s _self;
static int _self_is_used = 0;
#endif

/**
  * @brief Constructor
  */
ev_t ev_init(void)
{
#if S2M_USE_STATIC
  if (_self_is_used) return NULL;
  _self_is_used = 1;
  struct event_s *self = &_self;
  memset((void*)self, 0, sizeof(struct event_s));
#else
  struct event_s *self = calloc(1, sizeof(struct event_s));
  if (!self) return NULL;
#endif
  
  self->type = EV_NONE;
  self->active = false;
#if (EV_USE_THREADS)
  self->sem = semph_new();
  if (!self->sem) goto error;
#endif
  
  printf("[ev_init]\n");
  return self;

#if (EV_USE_THREADS)
  error:
#if S2M_USE_STATIC
  _self_is_used = 0;
#else
  free(self);
#endif //S2M_USE_STATIC
  printf("[ev_init] err\n");
  return NULL;
#endif //EV_USE_THREADS
}

/**
  * @brief Destructor
  */
void ev_del(ev_t self)
{
  assert(self);
  printf("[ev_del]\n");
#if (EV_USE_THREADS)
  semph_del(self->sem);
#else
  (void)self;
#endif
#if S2M_USE_STATIC
  _self_is_used = 0;
#else
  free(self);
#endif
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
