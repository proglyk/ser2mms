/**
  * @file   event.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   30-September-2025
  */

#ifndef  SER2MMS_EVENT_H
#define  SER2MMS_EVENT_H

#include "port_types.h"
#include <stdbool.h>

typedef struct event_s *event_t;
typedef event_t         ev_t;

typedef enum {
  EV_NONE, EV_RCVD, EV_EXEC, EV_SENT
} ev_type_t;

ev_t ev_init(void);
void ev_del(ev_t);
void ev_post(ev_t, ev_type_t);
bool ev_get(ev_t, ev_type_t *);
  
#endif