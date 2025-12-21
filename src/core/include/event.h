/**
* @file event.h
* @author Ilia Proniashin, msg@proglyk.ru
* @date 30-September-2025
*/

#ifndef SER2MMS_EVENT_H
#define SER2MMS_EVENT_H

#include "ser2mms_conf.h"
#include "port_types.h"
#include <stdbool.h>

// Allocation
#define EVENT_USE_STATIC                (0) //S2M_USE_STATIC

//#define EV_USE_THREADS (PORT_USE_THREADS)
#define EV_USE_THREADS                  (0)

typedef struct event_s *event_t;
typedef event_t ev_t;

typedef enum {
  EV_NONE, EV_RCVD, EV_EXEC, EV_SENT
} ev_type_t;

/**
* @brief Creates a new event instance
* @retval Pointer to event instance on success
* @retval NULL on allocation failure
*/
ev_t ev_new(void);

/**
* @brief Destroys event instance and releases resources
* @param self Pointer to this instance
*/
void ev_destroy(ev_t self);

/**
* @brief Posts (signals) an event with specified type
* @param self Pointer to this instance
* @param type Event type to post
*/
void ev_post(ev_t self, ev_type_t type);

/**
* @brief Retrieves pending event (blocks if using threads)
* @param self Pointer to this instance
* @param ptype Pointer to store received event type
* @retval true if event was retrieved
* @retval false if no event pending (non-threaded mode only)
*/
bool ev_get(ev_t self, ev_type_t *ptype);

#endif
