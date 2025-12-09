/**
  * @file   port_semph.c
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   04-October-2025
  */

#ifndef __unix__
#error "Should only be compiled under a unix system"
#endif

#include "port_semph.h"
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>

// Макрос PORT_SEMPH_USE_STATIC должен быть выкл. т.к. semph_t 
// не поддерживает статичной аллокации
#if (PORT_SEMPH_USE_STATIC)
#error "Macro 'PORT_SEMPH_USE_STATIC' must be disabled if you're using 'semph_t'"
#endif

struct semph_s {
  sem_t h;
};
  
/**
  * @brief Constructor
  */
semph_t semph_new(void)
{
  semph_t self = NULL;
  s32_t rc;
  
  self = calloc(1, sizeof(struct semph_s));
  if (!self) return NULL;
  
  rc = sem_init(&self->h, 0, 0);
  if (rc) goto exit;
  
  return self;
  
  exit:
  free(self);
  return NULL;
}

/**
  * @brief Destructor
  */
s32_t semph_del(semph_t self)
{
  assert(self);  
  s32_t rc = sem_destroy(&self->h);
  free(self);
  return rc;
}

/**
  * @brief ???
  */
s32_t semph_wait(semph_t self)
{
  assert(self);
  return sem_wait(&self->h);
}

/**
  * @brief ???
  */
s32_t semph_post(semph_t self)
{
  assert(self);
  return sem_post(&self->h);
}
