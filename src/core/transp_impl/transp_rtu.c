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

static void tick(void *);
static void recv(void *);
static void xmit(void *);
static s32_t msg_unpack(transp_t *tp);
static void msg_pack(transp_t *tp);

typedef enum {
  RECV_INIT, RECV_IDLE, RECV_ACT, RECV_ERR
} recv_sta_t;

typedef enum {
  XMIT_INIT, XMIT_IDLE, XMIT_ACT, XMIT_NREDE, XMIT_ERR
} xmit_sta_t;

struct transp_s
{
  tmr_t tmr;
  rs485_t stty;
  recv_sta_t recv_sta;
  xmit_sta_t xmit_sta;
  ev_t ev_rcvd;
  ev_t ev_xmit;
  u32_t id;
  ser_t ser;
  topmode_t mode; // TODO to move up
};

STATIC_DECLARE(TRANSP, struct transp_s);

//================================ PUBLIC API ==================================

/**
* @brief Constructor
*/
void *transp_init(__UNUSED int argc, __UNUSED int *pdata, __UNUSED void *argv,
                  __UNUSED void *irq, carg_fn_t fn1, subs_fn_t fn2, answ_fn_t fn3,
                  void *pld_api, u32_t mode, u32_t id, void *stty_init)
{
  ALLOC(TRANSP, struct transp_s, self, return NULL);
  self->id = id;
  self->mode = mode;
  self->recv_sta = RECV_INIT;
  self->xmit_sta = ((mode == MODE_SLAVE) ? (XMIT_INIT) : (XMIT_ACT));

  // timer
  self->tmr = tmr__init(S2M_TRANSP_TIMEOUT, (tmr_tick_t)tick, (void *)self);
  if (!self->tmr) goto error_0;

  // rs485
  rs485_fn_t fn_s = {
    .func_rcv = recv, .func_xmt = xmit, .pld = (void *)self
  };
  self->stty = rs485_new(stty_init, &fn_s);
  if (!self->stty) {
    printf("[transp_init] rs485_new() gave a FAIL\n");
    goto error_1;
  }

  // event
  self->ev_rcvd = ev_init();
  if (!self->ev_rcvd) goto error_2;
  self->ev_xmit = ev_init();
  if (!self->ev_xmit) goto error_3;

  // top layer
  self->ser = ser_new(mode, fn1, fn2, fn3, pld_api);
  if (!self->ser) goto error_4;

  //ser_set_cmd(self->ser, 1);
  return (void *)self;

  // cleanup created objects
error_4: ev_del(self->ev_xmit);
error_3: ev_del(self->ev_rcvd);
error_2: rs485_del(self->stty);
error_1: tmr__dis(self->tmr);
error_0: FREE(TRANSP, self);
  return NULL;
}

/**
* @brief Destructor
*/
void transp_del(__UNUSED int argc, void *opaque)
{
  transp_t *self = (transp_t *)opaque;
  assert(self);
  tmr__del(self->tmr);
  rs485_del(self->stty);
  ev_del(self->ev_rcvd);
  ev_del(self->ev_xmit);
  ser_destroy(self->ser);
  FREE(TRANSP, self);
}

/**
* @brief Start work
*/
void transp_run( transp_t *self )
{
  assert(self);
  //self->recv_sta = RECV_INIT;
  rs485_ena( self->stty, true, false );
  tmr__ena(self->tmr);
}

/**
* @brief Poll transport layer state machines
*/
int transp_poll(transp_t *tp)
{
  ev_type_t type;
  bool sta = false;
  s32_t rc;
  static u32_t counter = 0;

  //printf("[mb_tp__poll] tp->recv_sta is %d\n", tp->recv_sta);
  //printf("[mb_tp__poll] tp->xmit_sta is %d\n", tp->xmit_sta);

  // Polling
  rs485_poll(tp->stty);

  // if (tp->mode == MODE_SLAVE) {
  tmr__poll(tp->tmr);
  // }

  // Try to get rcvd event. If threads used blocking when no event available
  if (tp->mode == MODE_SLAVE) {
    sta = ev_get(tp->ev_rcvd, &type);
    if (sta) {
      switch ( type )
      {
        case EV_RCVD: {
          //
          rc = msg_unpack(tp);
#if (S2M_DEBUG)
          if (rc) {
            printf("[transp_poll] MB isn't ok\r\n");
          } else {
            msg_pack(tp);
            tp->xmit_sta = XMIT_ACT;
            rs485_ena(tp->stty, false, true);
            //ev_post( tp->ev_rcvd, EV_SENT );
            printf("[transp_poll] MB is ok\r\n");
          }
#endif //S2M_DEBUG
        } break;
        case EV_EXEC: {
          // eMBRTUSend()
          //ev_post( tp->ev_rcvd, EV_SENT );
        } break;
        case EV_SENT: {
          // eMBRTUSend()
          //tp->xmit_sta = XMIT_ACT;
          //rs485_ena(tp->stty, false, true);
        } break;
        case EV_NONE:
        default: break;
      }
    }
  }

  if (tp->mode == MODE_POLL) {
    // sta = ev_get(tp->ev_rcvd, &type);
    // if (sta) {
    //   printf("[mb_tp__poll] sta_ev_rcvd is %d\n", sta);
    //   if (type == EV_RCVD) {
    //     rc = msg_unpack(tp);
    //     #if (S2M_DEBUG)
    //     if (rc) printf("[transp_poll] MB isn't ok\n");
    //     else printf("[transp_poll] MB is ok\n");
    //     #endif //S2M_DEBUG
    //   } else {
    //     printf("[transp_poll] type is unknown\n");
    //   }
    // }

    // if (counter >= 4) {
    //   ev_post( tp->ev_xmit, EV_SENT );
    //   counter=0;
    // } else counter++;

    sta = ev_get(tp->ev_xmit, &type);
    if (sta) {
      printf("[mb_tp__poll] sta_ev_xmit is %d\n", sta);
      switch ( type ) {
        case EV_RCVD: {
        } break;
        case EV_EXEC: {
        } break;
        case EV_SENT: {
          //
          msg_pack(tp);
          //
          tp->xmit_sta = XMIT_ACT;
          rs485_ena(tp->stty, false, true);
        } break;
      }
    }
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
  recv((void*)self);
}

/**
* @brief Transmit interrupt wrapper
*/
void transp_xmit(transp_t *self)
{
  assert(self);
  xmit((void*)self);
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
* @brief Receive next byte
*/
static void recv(void *opaque)
{
  transp_t *self = (transp_t *)opaque;
  buf_rcvd_t pbuf;
  u8_t byte;
  bool rc;
  assert(self);

  pbuf = GET_RCVD(self->ser);
  rc = rs485_get(self->stty, &byte);
  if (rc) {
    printf("[recv] Received: %#X\n", byte);
    switch ( self->recv_sta )
    {
      case RECV_INIT: {
      } break;
      case RECV_IDLE: {
        self->recv_sta = RECV_ACT;
        pbuf->size = 0;
        pbuf->buf[pbuf->size++] = byte;
      } break;
      case RECV_ACT: {
        if( pbuf->size < BUFSIZE )
          pbuf->buf[pbuf->size++] = byte;
        else
          self->recv_sta = RECV_ERR;
      } break;
      case RECV_ERR: {
      } break;
    }
  } else {
    printf("[recv] Missed\r\n");
  }

  // update time_last // restart timer
  tmr__ena(self->tmr); // TODO tmr_set_prd(self->tmr, 600);
}

/**
* @brief Transmit next byte
*/
static void xmit(void *opaque)
{
  transp_t *self = (transp_t *)opaque;
  buf_xmit_t pbuf;
  static int count = 0;
  assert(self);

  pbuf = GET_XMIT(self->ser);
  switch ( self->xmit_sta )
  {
    case XMIT_INIT: {
    } break;
    case XMIT_IDLE: {
    } break;
    case XMIT_ACT: {
      if ( pbuf->pos < pbuf->size ) {
        rs485_put( self->stty, pbuf->buf[pbuf->pos++] );
      } else {
        rs485_ena_wait(self->stty, false);
        self->xmit_sta = XMIT_NREDE;
      }
    } break;
    case XMIT_NREDE: {
      rs485_ena(self->stty, true, false);
      self->xmit_sta = XMIT_IDLE;
    } break;
    case XMIT_ERR: {
    } break;
  }
}

/**
* @brief Timer periodic tick routine
*/
static void tick(void *opaque)
{
  transp_t *self = (transp_t *)opaque;
  assert(self);

#if (S2M_DEBUG)
  printf("[tick] Timer tick!\r\n");
#endif //S2M_DEBUG

  switch ( self->recv_sta )
  {
    case RECV_INIT: {
      //ev_post( self->ev_rcvd, EV_RDY ); //TODO why is this needed?
    } break;
    case RECV_ACT: {
      ev_post( self->ev_rcvd, EV_RCVD ); //TODO restore to proper place in RECV_ACT
#if (S2M_DEBUG)
      printf("[tick] new event\n");
#endif //S2M_DEBUG
    } break;
    //case RECV_IDLE:
    //case RECV_ERR:
    default: {
      printf("[tick] Ticked at the wrong moment\n");
    } break;
  }

  // Always disable timer
  tmr__dis(self->tmr);
  self->recv_sta = RECV_IDLE;
}

/**
* @brief Unpack and validate received message
*/
static s32_t msg_unpack(transp_t *self)
{
  buf_rcvd_t pbuf = GET_RCVD(self->ser);
  if (pbuf->size < 3) return -1;
  pbuf->pos = 0;

  // Check for address
  if (pbuf->buf[pbuf->pos++] != 12 ) {
    printf("[msg_unpack] address is wrong\r\n");
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
    printf("[msg_unpack] CRC is wrong\r\n");
    return -1;
  }

  // Call the top layer
  s32_t rc = ser_in_parse(self->ser);
  if (rc < 0) {
    printf("[msg_unpack] Can't parse\r\n");
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

#endif//MMS2SER_TRANSP_USE_RTU
