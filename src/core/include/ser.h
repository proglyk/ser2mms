/**
  * @file   ser.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   30-September-2025
  */

#ifndef SER2MMS_SER_H
#define SER2MMS_SER_H

#include "ser2mms_conf.h"
#include "types.h"
#include "port_types.h"

//#define MB_SER_PDU_SIZE_MAX             (16) // Maximum size of a Modbus RTU frame.
#define REGS_NUM                        (125)
// Размер таблицы (число) доступных для чтения и записи (R/W) регистров.
#define MB_INPUT_SIZE                   REGS_NUM
#define MB_HOLD_SIZE                    REGS_NUM
// Размер буферов
#define BUFSIZE                         (2*REGS_NUM)

// Short names
#define MB_RCV_BUF(S)                   (GET_RCVD(S)->buf)
#define MB_RCV_POS(S)                   (GET_RCVD(S)->pos)
#define MB_RCV_SIZE(S)                  (GET_RCVD(S)->size)
#define MB_XMT_BUF(S)                   (GET_XMIT(S)->buf)
#define MB_XMT_POS(S)                   (GET_XMIT(S)->pos)
#define MB_XMT_SIZE(S)                  (GET_XMIT(S)->size)
// Short names for func declarations
//#define buf_t                           ser_buf_t // TODO слишком общно
// Short names for func declarations
#define GET_RCVD(S)                     ser_get_buf_rcvd(S)
#define GET_XMIT(S)                     ser_get_buf_xmit(S)

// Struct decratation
struct ser_buf_s {
  u8_t *p;
  u32_t pos;
  u32_t size;
};
struct buf_rcvd_s {
  u8_t  buf[BUFSIZE];
  u32_t pos;
  u32_t size;
};
struct buf_xmit_s {
  u8_t  buf[BUFSIZE+10];
  u32_t pos;
  u32_t size;
};

// Type decratation
typedef struct buf_rcvd_s *buf_rcvd_t;
typedef struct buf_xmit_s *buf_xmit_t;
typedef struct ser_buf_s  *ser_buf_t;
typedef struct ser_s      *ser_t;

// Func declarations
ser_t ser_new(topmode_t, carg_fn_t, subs_fn_t, answ_fn_t, void *);
void  ser_del(ser_t);
void  ser_set_cmd(ser_t, u32_t);
s32_t ser_in_parse(ser_t);
void  ser_out_build(ser_t);
buf_rcvd_t ser_get_buf_rcvd(ser_t);
buf_xmit_t ser_get_buf_xmit(ser_t);

#endif
