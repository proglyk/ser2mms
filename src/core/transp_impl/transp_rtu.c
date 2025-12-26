/**
 * @file transp_rtu.c
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 10-October-2025
 * 
 * RTU transport layer implementation.
 */

#include "transp.h"
#include "alloc.h"
#include "event.h"
#include "byteops.h"
#include "ser.h"
#include "port_tmr.h"
#include "port_rs485.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#if (S2M_USE_TRANSP_RTU)

/** Receiver states. */
typedef enum {
  RECV_INIT,  // Initialization
  RECV_IDLE,  // Idle
  RECV_ACT    // Active reception
} recv_sta_t;

/** Transmitter states. */
typedef enum {
  XMIT_INIT,  // Initialization
  XMIT_IDLE,  // Idle
  XMIT_ACT,   // Active transmission
  XMIT_ERR    // Error
} xmit_sta_t;

/** Internal transport layer structure. */
struct transp_s
{
  rs485_t stty;        // RS485 interface
  recv_sta_t recv_sta; // Receiver state
  xmit_sta_t xmit_sta; // Transmitter state
  ev_t ev_rcvd;        // Receive event
  ev_t ev_xmit;        // Transmit event
  u32_t id;            // Device address identifier
  ser_t ser;           // Serial protocol handler
  ser_mode_t mode;     // Operation mode
};

STATIC_DECLARE(TRANSP, struct transp_s);

// Private function declarations
extern u16_t crc16(const u8_t *, u16_t);

static void recv_impl(void *, u32_t);
static void xmit_impl(void *);
static s32_t msg_unpack(transp_t *tp);
static void msg_pack(transp_t *tp);

// Public interface function definitions

// Basic functions

/**
 * 'transp' object constructor.
 */
void *transp_new(__UNUSED int argc, __UNUSED int *pdata, __UNUSED void *argv,
                 __UNUSED void *irq, void *pld_api, u32_t mode, u32_t id, void *stty_init)
{
  ALLOC(TRANSP, struct transp_s, self, return NULL);

  self->id = id;
  self->mode = mode;
  self->recv_sta = RECV_INIT;
  self->xmit_sta = XMIT_INIT;

  // RS485 initialization
  rs485_fn_t fn_s = {
    .func_rcv = recv_impl, .func_xmt = xmit_impl, .pld = (void *)self
  };
  self->stty = rs485_new(stty_init, &fn_s);
  if (!self->stty) {
    printf("[transp_init] rs485_new() returned FAIL\n");
    goto error_0;
  }

  // Event initialization
  self->ev_rcvd = ev_new();
  if (!self->ev_rcvd) goto error_1;

  self->ev_xmit = ev_new();
  if (!self->ev_xmit) goto error_2;

  // Upper layer initialization
  self->ser = ser_new(mode, pld_api);
  if (!self->ser) goto error_3;

  return (void *)self;

  // Cleanup created objects on error
error_3: ev_destroy(self->ev_xmit);
error_2: ev_destroy(self->ev_rcvd);
error_1: rs485_del(self->stty);
error_0: FREE(TRANSP, self);
  return NULL;
}

/**
 * 'transp' object destructor.
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
 * Start operation.
 */
void transp_run(transp_t *self)
{
  assert(self);
  self->recv_sta = RECV_IDLE;
  rs485_ena(self->stty, true, false);
}

/**
 * Poll transport layer.
 */
int transp_poll(transp_t *tp)
{
  ev_type_t type;

  // Poll RS485 receiver
  rs485_poll_rx(tp->stty);

  // Process events depending on mode
  switch (tp->mode)
  {
    case MODE_SLAVE: {
      // Receive event (message from master)
      if (ev_get(tp->ev_rcvd, &type)) {
        if (type == EV_RCVD) {
          tp->recv_sta = RECV_IDLE;
          if (msg_unpack(tp) == 0) {
            msg_pack(tp);
            tp->xmit_sta = XMIT_ACT;
            rs485_ena(tp->stty, false, true);
            rs485_poll_tx(tp->stty);
          }
        }
      }
    } break;

    case MODE_POLL: {
      // Receive event (response message from slave)
      if (ev_get(tp->ev_rcvd, &type)) {
        if (type == EV_RCVD) {
          msg_unpack(tp);
        }
      }

      // Transmit event (command from user)
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

// Helper functions

/**
 * Timer event handler.
 */
void transp_tick(transp_t *self)
{
  assert(self);
  ev_post(self->ev_xmit, EV_SENT);
}

/**
 * Receive interrupt handler.
 */
void transp_recv(transp_t *self)
{
  assert(self);
  recv_impl((void*)self, 0);
}

/**
 * Transmit interrupt handler.
 */
void transp_xmit(transp_t *self)
{
  assert(self);
  xmit_impl((void*)self);
}

/**
 * Getter for 'ser' module pointer.
 */
void *transp_get_top(transp_t *self)
{
  assert(self);
  return (void *)self->ser;
}

/**
 * Device ID setter.
 */
void transp_set_id(transp_t *self, u32_t id)
{
  assert(self);
  self->id = id;
}

// Private function definitions

// Parse incoming, build outgoing messages

/**
 * Unpack and validate received message.
 * Checks address, CRC checksum and calls upper layer parsing.
 */
static s32_t msg_unpack(transp_t *self)
{
  buf_rcvd_t pbuf = GET_RCVD(self->ser);

  if (pbuf->size < 3) {
    printf("[msg_unpack] size < 3\n");
    return -1;
  }

  pbuf->pos = 0;

  // Address check
  if (pbuf->buf[pbuf->pos++] != 12) {
    printf("[msg_unpack] invalid address\n");
    return -1;
  }

  // CRC checksum verification
  u16_t crc = crc16(pbuf->buf, pbuf->size-2);
#if (CRC_YURA)&&(!CRC_MODBUS)
  if (crc != B_TO_S(pbuf->buf[pbuf->size-2], pbuf->buf[pbuf->size-1]))
#elif (CRC_MODBUS)&&(!CRC_YURA)
  if (crc != B_TO_S(pbuf->buf[pbuf->size-1], pbuf->buf[pbuf->size-2]))
#else
  #error "Please define any CRC type"
#endif
  {
    printf("[msg_unpack] invalid CRC\n");
    return -1;
  }

  // Call upper layer
  s32_t rc = ser_in_parse(self->ser);
  if (rc < 0) {
    printf("[msg_unpack] failed to parse message\n");
    return -1;
  }

  return 0;
}

/**
 * Pack message for transmission with CRC.
 * Builds message with address, calls upper layer and adds CRC.
 */
static void msg_pack(transp_t *self)
{
  buf_xmit_t pbuf = GET_XMIT(self->ser);

  pbuf->pos = 0;
  pbuf->size = 0;

  // Set address
  pbuf->buf[pbuf->size++] = self->id;

  // Call upper layer
  ser_out_build(self->ser);

  // Calculate and add CRC
  u16_t crc = crc16(pbuf->buf, pbuf->size);
#if (CRC_YURA)&&(!CRC_MODBUS)
  u8_t *ptr = pbuf->buf + pbuf->size;
  S_TO_PB(ptr, crc);
  pbuf->size += 2;
#elif (CRC_MODBUS)&&(!CRC_YURA)
  S_TO_swPB((pbuf->buf + pbuf->size), crc);
  pbuf->size += 2;
#else
  #error "Please define any CRC type"
#endif
}

// Functor implementation for receive/transmit via rs485 module

/**
 * Receive next byte.
 * Processes received bytes and forms incoming message.
 */
static void recv_impl(void *opaque, u32_t len)
{
  u8_t byte;
  assert(opaque);

  transp_t *self = (transp_t *)opaque;
  buf_rcvd_t pbuf = GET_RCVD(self->ser);

  switch (self->recv_sta)
  {
    case RECV_IDLE:
      self->recv_sta = RECV_ACT;
      pbuf->size = 0;
      __FALLTHROUGH; // No break - continue processing in RECV_ACT

    case RECV_ACT: {
      for (u32_t i=0; i<len; i++) {
        if (rs485_get(self->stty, &byte)) {
          if (pbuf->size < BUFSIZE) {
            pbuf->buf[pbuf->size++] = byte;
          }
        }
      }

      if (pbuf->size >= IN_MSG_SIZE_SLAVE) {
        ev_post(self->ev_rcvd, EV_RCVD);
      }
    } break;
  }
}

/**
 * Transmit next byte.
 * Transmits bytes from buffer via RS485 interface.
 */
static void xmit_impl(void *opaque)
{
  assert(opaque);
  transp_t *self = (transp_t *)opaque;
  buf_xmit_t pbuf = GET_XMIT(self->ser);

  switch (self->xmit_sta)
  {
    case XMIT_ACT: {
      if (pbuf->pos < pbuf->size) {
        rs485_put(self->stty, pbuf->buf[pbuf->pos++]);
      } else {
        rs485_ena(self->stty, true, false);
        self->xmit_sta = XMIT_IDLE;
      }
    } break;
  }
}

#endif
