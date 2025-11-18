/**
  * @file   port_thread.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   28-September-2025
  */

#ifndef PORT_THREAD_H
#define PORT_THREAD_H

#include "port_conf.h"
#include "port_types.h"

#define THREAD_USE_STATIC               (0) //PORT_USE_STATIC

typedef struct thread_s *thread_t;
//typedef struct thread_s *thread_inst_t;

thread_t thread_new(const u8_t *, void *(*)(void *), void *);
void  thread_del(thread_t);
s32_t thread_kill(thread_t, s32_t);
void  thread_exit( void );
void  thread_sleep( u32_t );

#endif //PORT_THREAD_H
