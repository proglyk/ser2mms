/**
  * @file   port_tmr.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   22-September-2025
  */

#ifndef  PORT_TMR_H
#define  PORT_TMR_H

#include "port_conf.h"
#include "port_types.h"
#include <stdbool.h>

#define TMR_USE_STATIC                  (0) //PORT_USE_STATIC

typedef struct tmr_s *tmr_t;
typedef void (*tmr_tick_t)(void *);
typedef void  *tmr_pld_t;

//new
tmr_t tmr__init(u32_t, tmr_tick_t, void *);
void  tmr__del( tmr_t );
void  tmr__poll(tmr_t);
s32_t tmr__ena(tmr_t);
void  tmr__dis(tmr_t);

// shared


// stm32 only
bool  tmr_is_ena(tmr_t);
void  tmr_set_prd(tmr_t, u32_t);
void  tmr_set_cnt(tmr_t, u32_t);
void *tmr_get_h(tmr_t);
// linux only

  
#endif //PORT_TMR_H