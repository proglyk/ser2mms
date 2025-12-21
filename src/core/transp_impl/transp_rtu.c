/**
* @file transp_rtu.c
* @author Ilia Proniashin, msg@proglyk.ru
* @date 10-October-2025
*/

#include "transp.h"
#include "alloc.h"
#include "event.h"
#include "macro.h"
#include "ser.h"
#include "port_tmr.h"
#include "port_rs485.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if (S2M_USE_TRANSP_RTU)

extern u16_t crc16(const u8_t *, u16_t);
static void  tick_impl(void *);
static void  recv_impl(void *, u32_t);
static void  xmit_impl(void *);
static s32_t msg_unpack(transp_t *tp);
static void  msg_pack(transp_t *tp);

typedef enum {
  RECV_INIT, RECV_IDLE, RECV_ACT
} recv_sta_t;

typedef enum {
  XMIT_INIT, XMIT_IDLE, XMIT_ACT, XMIT_ERR
} xmit_sta_t;

struct transp_s
{
  rs485_t stty;
  recv_sta_t recv_sta;
  xmit_sta_t xmit_sta;
  ev_t  ev_rcvd;
  ev_t  ev_xmit;
  u32_t id;
  ser_t ser;
  topmode_t mode; // TODO to move up
};
STATIC_DECLARE(TRANSP, struct transp_s);
int s2m_toggle = 0;

//================================ PUBLIC API ==================================

/**
* @brief Constructor
*/
void *transp_new(__UNUSED int argc, __UNUSED int *pdata, __UNUSED void *argv,
                  __UNUSED void *irq, void *pld_api, u32_t mode, u32_t id, void *stty_init)
{
  ALLOC(TRANSP, struct transp_s, self, return NULL);
  self->id = id;
  self->mode = mode;
  self->recv_sta = RECV_INIT;
  self->xmit_sta = XMIT_INIT;
  //self->xmit_sta = ((mode == MODE_SLAVE) ? (XMIT_INIT) : (XMIT_ACT));

  // rs485
  rs485_fn_t fn_s = {
    .func_rcv = recv_impl, .func_xmt = xmit_impl, .pld = (void *)self
  };
  self->stty = rs485_new(stty_init, &fn_s);
  if (!self->stty) {
    printf("[transp_init] rs485_new() gave a FAIL\n");
    goto error_0;
  }

  // event
  self->ev_rcvd = ev_new();
  if (!self->ev_rcvd) goto error_1;
  self->ev_xmit = ev_new();
  if (!self->ev_xmit) goto error_2;

  // top layer
  self->ser = ser_new(mode, pld_api);
  if (!self->ser) goto error_3;

  //ser_set_cmd(self->ser, 1);
  return (void *)self;

  // cleanup created objects
  error_3: ev_destroy(self->ev_xmit);
  error_2: ev_destroy(self->ev_rcvd);
  error_1: rs485_del(self->stty);
  error_0: FREE(TRANSP, self);
  return NULL;
}

/**
* @brief Destructor
*/
void transp_destroy(__UNUSED int argc, void *opaque)
{
  transp_t *self = (transp_t *)opaque;
  assert(self);
  rs485_del(self->stty);
  ev_destroy(self->ev_rcvd);
  ev_destroy(self->ev_xmit);
  ser_destroy(self->ser);
  FREE(TRANSP, self);
}

/**
* @brief Start work
*/
void transp_run( transp_t *self )
{
  assert(self);
  self->recv_sta = RECV_IDLE;
  rs485_ena( self->stty, true, false );
}

/**
* @brief Poll transport layer state machines
*/
int transp_poll(transp_t *tp)
{
  ev_type_t type;

  // 1.
  rs485_poll_rx(tp->stty);

  // 2.
  switch (tp->mode)
  {
    case MODE_SLAVE: {
      // Событие на прием (сообщение от ведущего)
      if (ev_get(tp->ev_rcvd, &type)) {
        if (type == EV_RCVD) {
          tp->recv_sta = RECV_IDLE;
          // s2m_toggle ^= 1;
          if (msg_unpack(tp) == 0) {
            // printf("[transp_poll] msg_unpack == 0\n");
            msg_pack(tp);
            tp->xmit_sta = XMIT_ACT;
            rs485_ena(tp->stty, false, true);
            rs485_poll_tx(tp->stty);
          }
        }
      }
    } break;

    case MODE_POLL: {
      // Событие на прием (ответное сообщение от ведомого)
      if (ev_get(tp->ev_rcvd, &type)) {
        if (type == EV_RCVD) {
          msg_unpack(tp);
        }
      }
      // Событие на передачу (команда от пользователя)
      if (ev_get(tp->ev_xmit, &type)) {
        if (type == EV_SENT) {
          msg_pack(tp);
          tp->xmit_sta = XMIT_ACT;
          rs485_ena(tp->stty, false, true);
        }
      }
    } break;
  }

  return 0;
}

/**
* @brief Timer tick wrapper
*/
void transp_tick(transp_t *self)
{
  assert(self);
  //transp_t *tp = (transp_t *)self;
  //tick(argv);
  ev_post( self->ev_xmit, EV_SENT );
}

/**
* @brief Receive interrupt wrapper
*/
void transp_recv(transp_t *self)
{
  assert(self);
  recv_impl((void*)self, 0);
}

/**
* @brief Transmit interrupt wrapper
*/
void transp_xmit(transp_t *self)
{
  assert(self);
  xmit_impl((void*)self);
}

/**
* @brief Get top layer handler
*/
void *transp_get_top(transp_t *self)
{
  assert(self);
  return (void *)self->ser;
}

/**
* @brief Set device address identifier
*/
void transp_set_id(transp_t *self, u32_t id)
{
  assert(self);
  self->id = id;
}

//=============================== PRIVATE API ==================================

/**
* @brief Unpack and validate received message
*/
static s32_t msg_unpack(transp_t *self)
{
  buf_rcvd_t pbuf = GET_RCVD(self->ser);
  if (pbuf->size < 3) {
    printf("[msg_unpack] size < 3\n");
    return -1;
  }
  pbuf->pos = 0;

  // Check for address
  if (pbuf->buf[pbuf->pos++] != 12 ) {
    printf("[msg_unpack] address is wrong\n");
    return -1;
  }

  // Check for valid CRC
  u16_t crc = crc16(pbuf->buf, pbuf->size-2);
#if (CRC_YURA)&&(!CRC_MODBUS)
  if (crc != B_TO_S( pbuf->buf[pbuf->size-2], pbuf->buf[pbuf->size-1] ))
#elif (CRC_MODBUS)&&(!CRC_YURA)
  if (crc != B_TO_S( pbuf->buf[pbuf->size-1], pbuf->buf[pbuf->size-2] ))
#else
  #error "Please define any CRC type"
#endif
  {
    printf("[msg_unpack] CRC is wrong\n");
    return -1;
  }

  // Call the top layer
  s32_t rc = ser_in_parse(self->ser);
  if (rc < 0) {
    printf("[msg_unpack] Can't parse\n");
    return -1;
  }

  return 0;
}

/**
* @brief Pack message for transmission with CRC
*/
static void msg_pack(transp_t *self)
{
  buf_xmit_t pbuf = GET_XMIT(self->ser);
  pbuf->pos = 0;
  pbuf->size = 0;

  // Set address
  pbuf->buf[pbuf->size++] = self->id;

  // Call the top level
  ser_out_build(self->ser);

  // CRC
  u16_t crc = crc16(pbuf->buf, pbuf->size);
#if (CRC_YURA)&(!CRC_MODBUS)
  u8_t *ptr = pbuf->buf + pbuf->size;
  S_TO_PB(ptr, crc);
  pbuf->size += 2;
#elif (CRC_MODBUS)&(!CRC_YURA)
  S_TO_swPB( (pbuf->buf + pbuf->size), crc );
  pbuf->size += 2;
#else
  #error "Please define any CRC type"
#endif
}

/**
* @brief Receive next byte
*/
static void recv_impl(void *opaque, u32_t len)
{
  u8_t byte;
  assert(opaque);

  transp_t *self = (transp_t *)opaque;
  buf_rcvd_t pbuf = GET_RCVD(self->ser);

  switch ( self->recv_sta )
  {
    case RECV_IDLE:
      self->recv_sta = RECV_ACT;
      pbuf->size = 0;
    case RECV_ACT: {
      for (u32_t i=0; i<len; i++) {
        if (rs485_get(self->stty, &byte)) {
          if( pbuf->size < BUFSIZE ) {
            pbuf->buf[pbuf->size++] = byte;
          }
        }
      }
      if (pbuf->size >= IN_MSG_SIZE_SLAVE) {
        ev_post( self->ev_rcvd, EV_RCVD );
      }
    } break;
  }
}

/**
* @brief Transmit next byte
*/
static void xmit_impl(void *opaque)
{
  assert(opaque);

  transp_t *self = (transp_t *)opaque;
  buf_xmit_t pbuf = GET_XMIT(self->ser);

  switch ( self->xmit_sta )
  {
    case XMIT_ACT: {
      if ( pbuf->pos < pbuf->size ) {
        rs485_put( self->stty, pbuf->buf[pbuf->pos++] );
      } else {
        rs485_ena(self->stty, true, false);
        self->xmit_sta = XMIT_IDLE;
      }
    } break;
  }
}

#endif//MMS2SER_TRANSP_USE_RTU
