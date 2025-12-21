/**
  * @file   ser2mms.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   09-October-2025
  */

#include "ser2mms.h"
#if (S2M_USE_THREADS)
#include "port_thread.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void *poll(void *);

struct ser2mms_s {
  transp_t *tp;
  void     *ied;
#if (S2M_USE_THREADS)
  thread_t  thread;
#endif
};

#if S2M_USE_STATIC
static struct ser2mms_s _self;
static int _self_is_used = 0;
#endif

extern int running;

//================================ PUBLIC API ==================================


s2m_t *ser2mms_new(void *ied, u32_t mode, u32_t id, void *stty_init)
{
#if S2M_USE_STATIC
  if (_self_is_used) return NULL;
  _self_is_used = 1;
  struct ser2mms_s *self = &_self;
  memset((void*)self, 0, sizeof(struct ser2mms_s));
#else
  struct ser2mms_s *self = calloc(1, sizeof(struct ser2mms_s));
  if (!self) return NULL;
#endif

  self->ied = ied;
  // Transport layer
  self->tp = transp_new(0, NULL, NULL, NULL, (void *)self,
                         mode, id, stty_init);
  if (!self->tp) {
    //printf("[s2m_create] tp is null\n");
    goto error; //assert(self->tp);
  }
  
  return self;
  
  error:
#if S2M_USE_STATIC
  _self_is_used = 0;
#else
  free(self);
#endif
  return NULL;
}


s32_t ser2mms_run(s2m_t *self)
{
  assert(self);
#if (S2M_USE_THREADS)
  self->thread = thread_new((const u8_t *)"srv", &poll, (void *)self->tp);
  if (!self->thread) {
    ser2mms_stop(self);
    return -1;
  }
#endif
  transp_run(self->tp);
  return 0;
}

void ser2mms_poll(s2m_t *self)
{
  assert(self);
#if (!S2M_USE_THREADS)
  poll((void *)self->tp);
#endif
}

void ser2mms_stop(s2m_t *self)
{
  assert(self);
#if (S2M_USE_THREADS)
  thread_del(self->thread);
#endif
  transp_destroy(0, (void *)self->tp);
#if S2M_USE_STATIC
  _self_is_used = 0;
#else
  free(self);
#endif
}

void ser2mms_test_tick(s2m_t *self)
{
  assert(self);
  transp_tick(self->tp);
}

void ser2mms_set_cmd(s2m_t *self, u32_t cmd)
{
  assert(self);
  ser_t top = (ser_t)transp_get_top(self->tp);
  ser_set_cmd(top, cmd);
}

void ser2mms_set_id(s2m_t *self, u32_t id)
{
  assert(self);
  transp_set_id(self->tp, id);
}

void *ser2mms_get_ied(s2m_t *self)
{
  assert(self);
  return self->ied;
}

void __WEAK ser2mms_set_time(uint32_t *epoch, uint32_t *usec)
{
  
}

void __WEAK ser2mms_read_carg(void *opaque, u16_t *carg_buf, __UNUSED u32_t carg_len, u8_t ds, u8_t page)
{
  
}

void __WEAK ser2mms_read_subs(void *opaque, prm_t *subs_buf, __UNUSED u32_t subs_len)
{
  
}

void __WEAK ser2mms_write_answer(void *argv, u16_t *answ_buf, u32_t *answ_len)
{

}

#if 0
void *mb__get_ser_h(s2m_t *self)
{
  return mb_tp__get_ser_h((void *)self->tp);
}

void *mb__get_tim_h(s2m_t *self)
{
  return mb_tp__get_tim_h((void *)self->tp);
}

void mb__recv_cb(s2m_t *self)
{
  mb_tp__recv((void *)self->tp);
}

void mb__tick_cb(s2m_t *self)
{
  mb_tp__tick((void *)self->tp);
}

void mb__test_recv(s2m_t *self)
{
  mb_tp__recv((void *)self->tp);
}
#endif

//=============================== PRIVATE API ==================================

/**
  * @brief  Do a periodic calls (polling)
  * @param  opaque - Opaque pointer to Transport layer object
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
  thread_exit();   // Завершаем поток
#else
  transp_poll(tp);
#endif
  return NULL;
}
