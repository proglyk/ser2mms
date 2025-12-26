/**
 * @file event.h
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 30-September-2025
 * 
 * Event mechanism interface.
 * Provides simple event mechanism for synchronization between
 * system components with multithreading support via semaphores.
 */

#ifndef SER2MMS_EVENT_H
#define SER2MMS_EVENT_H

#include "ser2mms_conf.h"
#include "port_types.h"
#include <stdbool.h>

/** Memory allocation. */
#define EVENT_USE_STATIC (0)

/** Thread usage. */
#define EV_USE_THREADS (0)

/** Pointer type to event object. */
typedef struct event_s *event_t;

/** Type alias for brevity. */
typedef event_t ev_t;

/**
 * Event types.
 */
typedef enum {
  EV_NONE,  // No event
  EV_RCVD,  // Data receive event
  EV_EXEC,  // Execution event
  EV_SENT   // Data send event
} ev_type_t;

/**
 * Event object constructor.
 * Creates a new event instance with semaphore initialization (if threads are used).
 * 
 * @return pointer to event instance on success, NULL on allocation error
 */
ev_t ev_new(void);

/**
 * Event object destructor.
 * Destroys event instance and frees all resources including semaphore.
 * 
 * @param self pointer to instance
 */
void ev_destroy(ev_t self);

/**
 * Post event.
 * Sets event type and signals its occurrence.
 * In multithreaded mode releases semaphore.
 * 
 * @param self pointer to instance
 * @param type event type to post
 */
void ev_post(ev_t self, ev_type_t type);

/**
 * Get event.
 * Retrieves pending event. In multithreaded mode blocks until event appears,
 * in single-threaded mode returns immediately.
 * 
 * @param self pointer to instance
 * @param ptype pointer to store received event type
 * @return true if event was retrieved, false if no events pending (single-threaded mode only)
 */
bool ev_get(ev_t self, ev_type_t *ptype);

#endif
