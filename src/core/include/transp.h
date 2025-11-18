/**
  * @file   transp.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   30-September-2025
  */

#ifndef  SER2MMS_TRANSP_H
#define  SER2MMS_TRANSP_H

#include "ser2mms_conf.h"
#include "types.h"
#include "port_types.h"

typedef struct transp_s transp_t;
//typedef struct transp_s transp_ptr_t;
//typedef transp_t        transp_t;

int   transp_poll(transp_t *);
void *transp_init(int, int *, void *, void *, carg_fn_t, subs_fn_t, answ_fn_t,
                  void *, u32_t, u32_t, void *);
void  transp_run( transp_t * );
void  transp_del(int, void *);
void *transp_get_tim_h( transp_t * );
void *transp_get_ser_h( transp_t * );
void  transp_recv(transp_t *); // вход прерывания UARTx rcv
void  transp_xmit(transp_t *); // вход прерывания UARTx xmt
void  transp_tick(transp_t *); // вход прерывания TIMx
void *transp_get_top(transp_t *);
void  transp_set_id(transp_t *, u32_t);

//TEST
void mb_tp__tick(void*);

#endif