/**
 * @file port_thread.c
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 16-November-2025
 */

// #ifndef USE_FREERTOS
// #error "Should only be compiled with FreeRTOS enabled"
// #endif

#include "port_thread.h"
#include "port_alloc.h"
#include "FreeRTOS.h"
#include "task.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

// Макрос PORT_USE_THREADS должен быть включен для использования потоков
#if (!PORT_USE_THREADS)
#error "Macro 'PORT_USE_THREADS' must be enabled if you're using port_thread.c"
#endif

struct thread_s {
  TaskHandle_t tid;
  int state;
};

PORT_STATIC_DECLARE(THREAD, struct thread_s);

/**
 * @brief Run function addressed by pointer 'fn' under a new thread
 * @param name - Thread name (used for debugging in FreeRTOS)
 * @param fn - Function pointer to execute in new thread
 * @param pld - Payload pointer passed to function
 * @return Pointer to thread_t object or NULL on error
 */
thread_t thread_new(const u8_t *name, void *(*fn)(void *), void *pld)
{
  PORT_ALLOC(THREAD, struct thread_s, self, return NULL);
  
  // Создаем задачу FreeRTOS
  // Используем приведение типа функции для совместимости
  int rc = xTaskCreate(
    (TaskFunction_t)fn,                     // Функция задачи
    (const char *)(name ? name : "thread"), // Имя задачи
    4 * configMINIMAL_STACK_SIZE,           // Размер стека
    pld,                                    // Параметр задачи
    tskIDLE_PRIORITY + 1,                   // Приоритет
    &(self->tid)                            // Хэндл задачи
  );
  
  if (rc != pdPASS) goto exit;
  
  self->state = 1;
  printf("[thread_new] Created thread '%s'\n", name ? name : "unnamed");
  return self;

exit:
  PORT_FREE(THREAD, self);
  printf("[thread_new] Failed to create thread\n");
  return NULL;
}

/**
 * @brief Delete the running thread
 * @param self - Pointer to thread_t object
 */
void thread_del(thread_t self)
{
  assert(self);
  
  // Удаляем задачу FreeRTOS
  if (self->tid != NULL) {
    vTaskDelete(self->tid);
    self->tid = NULL;
  }
  
  printf("[thread_del]\n");
  
  PORT_FREE(THREAD, self);
}

/**
 * @brief Suspend a thread (FreeRTOS equivalent of signal-based control)
 * @param self - Pointer to thread_t object
 * @param sig - Signal number (ignored in FreeRTOS, kept for API compatibility)
 * @return 0 on success, -1 on error
 */
s32_t thread_kill(thread_t self, s32_t sig)
{
  assert(self);
  
  if (self->tid == NULL) return -1;
  
  // В FreeRTOS нет прямого аналога pthread_kill с сигналами
  // Используем suspend как наиболее близкий аналог
  // Для полной остановки используйте thread_del()
  vTaskSuspend(self->tid);
  
  return 0;
}

/**
 * @brief Exit current thread
 */
void thread_exit(void)
{
  // В FreeRTOS удаляем текущую задачу
  vTaskDelete(NULL);
}
