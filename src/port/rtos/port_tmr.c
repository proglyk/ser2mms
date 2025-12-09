/**
  * @file port_tmr.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date 16-November-2025
  */

// #ifndef USE_FREERTOS
// #error "Should only be compiled with FreeRTOS enabled"
// #endif

#include "port_tmr.h"
#include "port_alloc.h"
#include "FreeRTOS.h"
#include "task.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct tmr_s {
  bool enabled;
  u32_t timeout;
  TickType_t xTicksToWait;
  TimeOut_t xTimeOut;
  tmr_tick_t fn;
  tmr_pld_t pld;
};

PORT_STATIC_DECLARE(TMR, struct tmr_s);

/**
  * @brief Constructor
  * @param timeout - Response timeout, ms
  * @param fn - Callback function pointer
  * @param pld - Payload pointer
  * @retval Pointer to the object itself
  */
tmr_t tmr__init(u32_t timeout, tmr_tick_t fn, tmr_pld_t pld)
{
  PORT_ALLOC(TMR, struct tmr_s, self, return NULL);
  assert( fn && pld && ((timeout > 25) && (timeout < 10000)) );
  
  self->enabled = false;
  self->timeout = timeout;
  self->fn = fn;
  self->pld = pld;
  
  printf("[tmr__init]\n");
  return self;
}

/**
  * @brief Destructor
  * @param self - Pointer to the object itself
  */
void tmr__del(tmr_t self)
{
  assert(self);
  printf("[tmr__del]\n");
  tmr__dis(self);
  
  PORT_FREE(TMR, self);
}

/**
  * @brief Poll timer and call callback if timeout expired
  * @param self - Pointer to the object itself
  */
void tmr__poll(tmr_t self)
{
  assert(self);
  if (!self->enabled) return;
  
  // Проверяем истечение таймаута
  // xTaskCheckForTimeOut обновляет xTicksToWait и возвращает pdTRUE если таймаут истек
  if (xTaskCheckForTimeOut(&self->xTimeOut, &self->xTicksToWait) == pdTRUE) {
    //self->enabled = false; TODO delete
    if (self->fn) self->fn(self->pld);
  }
}

/**
  * @brief Enable timer
  * @param self - Pointer to the object itself
  * @retval 0 on success, -1 on error
  */
s32_t tmr__ena(tmr_t self)
{
  assert(self);
  
  // Конвертируем миллисекунды в тики FreeRTOS
  self->xTicksToWait = pdMS_TO_TICKS(self->timeout);
  
  // Сохраняем текущее состояние времени (точка отсчета)
  vTaskSetTimeOutState(&self->xTimeOut);
  
  self->enabled = true;
  return 0;
}

/**
 * @brief Disable timer
 * @param self - Pointer to the object itself
 */
void tmr__dis(tmr_t self)
{
  assert(self);
  self->enabled = false;
}
