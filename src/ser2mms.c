/**
* @file ser2mms.c
* @author Ilia Proniashin, msg@proglyk.ru
* @date 09-October-2025
*/

#include "ser2mms.h"
#include "alloc.h"

#if (S2M_USE_THREADS)
#include "port_thread.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Type definitions

// Internal ser2mms object structure.
struct ser2mms_s {
  transp_t *tp;  // Pointer to transport layer
  void *ied;  // Pointer to IED server
#if (S2M_USE_THREADS)
  thread_t thread;  // Worker thread descriptor
#endif
};

// Variable declarations
STATIC_DECLARE(SER2MMS, struct ser2mms_s);

// Global application running flag.
extern int running;

// Private function declarations
static void *poll(void *);

// Public interface function definitions

// Basic functions

/**
* Object constructor.
*/
s2m_t *ser2mms_new(void *ied, u32_t mode, u32_t id, void *stty_init)
{
  ALLOC(SER2MMS, struct ser2mms_s, self, return NULL);
  self->ied = ied;

  // Create transport layer
  self->tp = transp_new(0, NULL, NULL, NULL, (void *)self,
                        mode, id, stty_init);
  if (!self->tp) {
    goto error;
  }
  return self;

error:
  FREE(SER2MMS, self);
  return NULL;
}

/**
* Object destructor.
*/
void ser2mms_destroy(s2m_t *self)
{
  assert(self);
#if (S2M_USE_THREADS)
  thread_del(self->thread);
#endif
  transp_destroy(0, (void *)self->tp);
  FREE(SER2MMS, self);
}

/**
* Start operation.
*/
s32_t ser2mms_run(s2m_t *self)
{
  assert(self);
#if (S2M_USE_THREADS)
  self->thread = thread_new((const u8_t *)"srv", &poll, (void *)self->tp);
  if (!self->thread) {
    ser2mms_destroy(self);
    return -1;
  }
#endif
  transp_run(self->tp);
  return 0;
}

/**
* Polling function.
*/
void ser2mms_poll(s2m_t *self)
{
  assert(self);
#if (!S2M_USE_THREADS)
  poll((void *)self->tp);
#endif
}

// Functions with external implementation

/** For SLAVE mode. */

/** Set system time. */
void __WEAK ser2mms_set_time(uint32_t *epoch, uint32_t *usec)
{
  (void)epoch; (void)usec;
}

/** Read page parameters. */
void __WEAK ser2mms_read_page(const page_prm_t *buf, u8_t ds, u8_t page, void *opaque)
{
  (void)buf; (void)ds; (void)page; (void)opaque;
}

/** Read subscription array. */
void __WEAK ser2mms_read_subs(const sub_prm_t *buf, void *opaque)
{
  (void)buf; (void)opaque;
}

/** Write answer. */
void __WEAK ser2mms_write_answer(answ_prm_t *buf, u32_t *buf_len)
{
  (void)buf; (void)buf_len;
}

/** For POLL mode. */

/** Write page parameters. */
void __WEAK ser2mms_write_page(page_prm_t *buf, u32_t *buf_len, u8_t ds, u8_t page)
{
  (void)buf; (void)buf_len; (void)ds; (void)page;
}

/** Write page parameters. */
void __WEAK ser2mms_write_subs(sub_prm_t *buf, u32_t *buf_len)
{
  (void)buf; (void)buf_len;
}

// Helper functions

/**
* IED server instance getter.
*/
void *ser2mms_get_ied(s2m_t *self)
{
  assert(self);
  return self->ied;
}

/**
* Setter for transmitted command type (only in S2M_POLL mode).
*/
void ser2mms_set_cmd(s2m_t *self, u32_t cmd)
{
  assert(self);
  ser_t top = (ser_t)transp_get_top(self->tp);
  ser_set_cmd(top, cmd);
}

/**
* Device ID setter.
*/
void ser2mms_set_id(s2m_t *self, u32_t id)
{
  assert(self);
  transp_set_id(self->tp, id);
}

/**
* Test tick for debugging.
*/
void ser2mms_test_tick(s2m_t *self)
{
  assert(self);
  transp_tick(self->tp);
}

// Private function definitions

/**
* Thread function for periodic polling.
*
* @param opaque opaque pointer to transport layer object
*/
static void *poll(void *opaque)
{
  transp_t *tp = (transp_t *)opaque;
  assert(tp);

#if (S2M_USE_THREADS)
  do {
    transp_poll(tp);
  } while (running);
  printf("[poll] Caught condition '!running', exiting...\n");
  thread_exit();  // Terminate thread
#else
  transp_poll(tp);
#endif
  return NULL;
}
