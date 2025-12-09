/**
  * @file   port_thread.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   28-September-2025
  */

#include "port_thread.h"
#include "port_alloc.h"
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

// Макрос PORT_USE_STATIC должен быть выкл. т.к. библиотека 'c-periphery' 
// не поддерживает статичной аллокации
#if (!PORT_USE_THREADS)
#error "Macro 'PORT_USE_THREADS' must be enabled if you're using port_thread.c"
#endif

struct thread_s {
  pthread_t     tid;
  int           state;
};

PORT_STATIC_DECLARE(THREAD, struct thread_s);

// ============================= Публичные функции =============================

/**
  * @brief Run function addressed by pointer 'fn' under a new thread
  */
thread_t thread_new( __UNUSED const u8_t *name, void *(*fn)(void *),
                      void *pld )
{
  PORT_ALLOC(THREAD, struct thread_s, self, return NULL);

  int rc = pthread_create( &(self->tid), NULL, fn, pld );
  if (rc) goto exit;
  self->state = 1;
  return self;
  
  exit:
  PORT_FREE(THREAD, self);
  return NULL;
}

/**
  * @brief Delete the running thread
  */
void thread_del(thread_t self)
{
  assert(self);
  pthread_join(self->tid, NULL);
  PORT_FREE(THREAD, self);
}

/**
  * @brief Simple wrapper
  */
s32_t thread_kill( thread_t self, s32_t sig )
{
  assert(self);
  return pthread_kill(self->tid, sig);
}

/**
  * @brief Simple wrapper
  */
void thread_exit( void )
{
  return pthread_exit(NULL);
}
