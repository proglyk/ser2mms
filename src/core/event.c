/**
 * @file event.c
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 30-September-2025
 * 
 * Event mechanism implementation.
 */

#include "event.h"
#include "alloc.h"
#if (EV_USE_THREADS)
#include "port_semph.h"
#endif
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

/**
 * Internal event object structure.
 */
struct event_s {
  ev_type_t type;   // Event type
  bool active;      // Event active flag
#if (EV_USE_THREADS)
  semph_t sem;      // Semaphore for synchronization
#endif
};

STATIC_DECLARE(EVENT, struct event_s);

// Public interface function definitions

/**
 * Constructor.
 */
ev_t ev_new(void)
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
 * Destructor.
 */
void ev_destroy(ev_t self)
{
  assert(self);
#if (EV_USE_THREADS)
  semph_del(self->sem);
#endif
  FREE(EVENT, self);
}

/**
 * Post event.
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
 * Get event.
 */
bool ev_get(ev_t self, ev_type_t *ptype)
{
  assert(self);

#if (EV_USE_THREADS)
  // Wait for semaphore (blocking mode)
  semph_wait(self->sem);
  *ptype = self->type;
  self->active = false;
  return true;
#else
  // Check activity without blocking
  if (self->active) {
    *ptype = self->type;
    self->active = false;
    return true;
  }
  return false;
#endif
}
