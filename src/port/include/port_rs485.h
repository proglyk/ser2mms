/**
  * @file   port_rs485.h
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   14-November-2025
  */

#ifndef PORT_RS485_H
#define PORT_RS485_H

#include "port_conf.h"
#include "port_types.h"
#include <stdbool.h>

#define  RCVD_BUF_SIZE                  (128)
#define  XMIT_BUF_SIZE                  (128)

#define RS485_USE_STATIC                (0)
//#define RS485_USE_STATIC                PORT_USE_STATIC

typedef struct {
  void (*func_rcv)(void *);
  void (*func_xmt)(void *);
  void  *pld;
} rs485_fn_t;

//typedef void *rs485_init_t;

typedef struct rs485_s *rs485_t;
//typedef struct rs485_s *rs485_inst_t;

// linux, win32
rs485_t rs485_new(void *, rs485_fn_t *);
//rs485_t rs485_new(const char *, fn_t, fn_t, void *, const char *, u32_t);
void    rs485_del(rs485_t);
void    rs485_ena(rs485_t, bool, bool);
void    rs485_poll(rs485_t);
bool    rs485_get(rs485_t, u8_t *);
bool    rs485_put(rs485_t, u8_t);

// stm32
//void   *rs485_get_h(rs485_t);

#endif //PORT_RS485_H