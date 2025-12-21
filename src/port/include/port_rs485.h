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

#define RS485_USE_STATIC                (0) // PORT_USE_STATIC

typedef struct {
  void (*func_rcv)(void *, u32_t);
  void (*func_xmt)(void *);
  void  *pld;
} rs485_fn_t;

typedef struct rs485_s *rs485_t;

// linux, win32
rs485_t rs485_new(void *, rs485_fn_t *);
void    rs485_del(rs485_t);
void    rs485_ena(rs485_t, bool, bool);
void    rs485_poll_tx(rs485_t);
void    rs485_poll_rx(rs485_t);
bool    rs485_get(rs485_t, u8_t *);
bool    rs485_put(rs485_t, u8_t);
void    rs485_ena_wait(rs485_t self, bool wait_tx);

#endif //PORT_RS485_H
