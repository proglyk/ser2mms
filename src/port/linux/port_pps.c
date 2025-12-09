/**
 * @file port_pps.c
 * @brief 1PPS signal generator implementation
 * @date 30-November-2025
 */

#include "port_pps.h"
#include "port_thread.h"
#include "port_alloc.h"
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
#include "gpio.h"
#endif
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

// P9_18 = GPIO0_4
#define PPS_GPIO_CHIP    "/dev/gpiochip0"
#define PPS_GPIO_LINE    14
#define PPS_PULSE_WIDTH_NS   500000000  // 500 ms
#define PPS_PERIOD_NS        1000000000 // 1 second

static void *pps_thread(void *);

struct pps_s {
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  gpio_t       *gpio;
#endif
  thread_t      thread;
  volatile bool running;
};

PORT_STATIC_DECLARE(PPS, struct pps_s);

// ============================= Публичные функции =============================

/**
 * @brief Create new 1PPS generator instance
 */
pps_t pps_new(void)
{
  PORT_ALLOC(PPS, struct pps_s, self, return NULL);

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  self->gpio = gpio_new();
  if (!self->gpio) {
    fprintf(stderr, "pps_new: gpio_new() failed\n");
    goto err_alloc;
  }
  
  if (gpio_open(self->gpio, PPS_GPIO_CHIP, PPS_GPIO_LINE, GPIO_DIR_OUT) < 0) {
    fprintf(stderr, "pps_new: gpio_open() failed: %s\n", 
            gpio_errmsg(self->gpio));
    goto err_gpio;
  }
  
  // Initialize to LOW
  gpio_write(self->gpio, false);
#endif
  
  self->thread = NULL;
  self->running = false;
  
  return self;

err_gpio:
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  gpio_free(self->gpio);
#endif
err_alloc:
  PORT_FREE(PPS, self);
  return NULL;
}

/**
 * @brief Destroy 1PPS generator instance
 */
void pps_destroy(pps_t self)
{
  assert(self);
  
  // Stop thread if running
  if (self->running) {
    pps_stop(self);
  }

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  // Set GPIO to LOW before closing
  gpio_write(self->gpio, false);
  
  gpio_close(self->gpio);
  gpio_free(self->gpio);
#endif
  
  PORT_FREE(PPS, self);
}

/**
 * @brief Start 1PPS signal generation in separate thread
 */
s32_t pps_run(pps_t self)
{
  assert(self);
  
  if (self->running) {
    fprintf(stderr, "pps_run: already running\n");
    return -1;
  }
  
  self->running = true;
  
  self->thread = thread_new((const u8_t *)"pps_gen", 
                            pps_thread, 
                            self);
  if (!self->thread) {
    fprintf(stderr, "pps_run: thread_new() failed\n");
    self->running = false;
    return -1;
  }
  
  return 0;
}

/**
 * @brief Stop 1PPS signal generation
 */
void pps_stop(pps_t self)
{
  assert(self);
  
  if (!self->running) {
    return;
  }
  
  self->running = false;
  
  if (self->thread) {
    thread_del(self->thread);
    self->thread = NULL;
  }
}

// ============================ Статические функции ============================

/**
 * @brief 1PPS generation thread function
 */
static void *pps_thread(void *arg)
{
  pps_t self = (pps_t)arg;
  struct timespec ts_high, ts_low;
  
  assert(self);
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  assert(self->gpio);
#endif
  
  ts_high.tv_sec = 0;
  ts_high.tv_nsec = PPS_PULSE_WIDTH_NS;
  
  ts_low.tv_sec = 0;
  ts_low.tv_nsec = PPS_PERIOD_NS - PPS_PULSE_WIDTH_NS;
  
  printf("[pps_thread]: tick\n");
  
  while (self->running) {
    
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
    // Set HIGH
    if (gpio_write(self->gpio, true) < 0) {
      fprintf(stderr, "pps: gpio_write(HIGH) failed\n");
      break;
    }
#endif

    nanosleep(&ts_high, NULL);
    
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
    // Set LOW
    if (gpio_write(self->gpio, false) < 0) {
      fprintf(stderr, "pps: gpio_write(LOW) failed\n");
      break;
    }
#endif

    nanosleep(&ts_low, NULL);
  }
  
  thread_exit();
  return NULL;
}

