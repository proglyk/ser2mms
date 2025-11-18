/**
  * @file   ser.c
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   30-September-2025
  */

#include "ser.h"
#include "macro.h"
#include "types.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static s32_t decode_head(ser_t);
static void  encode_head(ser_t);
static void  process_pld(ser_t);
static void  compose_pld(ser_t);


struct ser_s {
  // Received
  cmd_t         cmd_rcvd; // Управление: 0 – обычный режим, 1 – передача времени
  struct buf_rcvd_s rcvd;
  
  // For transmit
  cmd_t         cmd_xmit;
  struct buf_xmit_s xmit;
  
  // General
  topmode_t     mode;
  u8_t          ds;
  u8_t          page;
  u16_t         cargvalue2[SER_PAGE_SIZE];
  prm_t         subsprm2[SER_NUM_SUBS];
  u16_t         answ_buf[SER_ANSW_SIZE];
  u32_t         answ_len;
  carg_fn_t     carg_fn;
  subs_fn_t     subs_fn;
  answ_fn_t     answ_fn;
  void         *pld_api;
};

#if S2M_USE_STATIC
static struct ser_s _self;
static int _self_is_used = 0;
#endif

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
ser_t ser_new(topmode_t mode, carg_fn_t carg_cb, subs_fn_t subs_cb, 
                       answ_fn_t answ_cb, void *pld_api)
{  
#if S2M_USE_STATIC
  if (_self_is_used) return NULL;
  _self_is_used = 1;
  struct ser_s *self = &_self;
  memset((void*)self, 0, sizeof(struct ser_s));
#else
  struct ser_s *self = calloc(1, sizeof(struct ser_s));
  if (!self) return NULL;
#endif
  self->mode = mode;
  self->ds = SER_MAX_DS_IDX;
  self->page = SER_MAX_PAGE_IDX;
  self->carg_fn = carg_cb;
  self->subs_fn = subs_cb;
  self->answ_fn = answ_cb;
  self->pld_api = pld_api;
  
  printf("[ser_create]\n");
  return self;
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
void ser_del(ser_t self)
{
  assert(self);
  printf("[ser_del]\n");
#if S2M_USE_STATIC
  _self_is_used = 0;
#else
  free(self);
#endif
}

/**
  * @brief  ?
  * @param  self - ?
  * @param  value - ?
  */
void ser_set_cmd(ser_t self, u32_t value)
{
  assert(self);
  self->cmd_xmit = value ? CMD_TIMESET : CMD_PARAMETERS;
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
s32_t ser_in_parse(ser_t self)
{  
  assert(self);
  
  u32_t size = (self->mode == MODE_SLAVE) ? IN_MSG_SIZE_SLAVE : 
                                            IN_MSG_SIZE_POLL;
  if (self->rcvd.size != size)
    return -1;
  if ( decode_head(self) < 0 ) {
    return -1;
  }
  process_pld(self);
  return 0;
  //return decode_head(self);
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
void ser_out_build(ser_t self)
{
  assert(self);
  
  encode_head(self);
  compose_pld(self);
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
static s32_t decode_head(ser_t self)
{ 
  u8_t  *buf;
  u32_t *pos;
  u16_t  cmd;
  
  assert(self);
  buf = self->rcvd.buf;
  pos = &self->rcvd.pos;
  
  switch (self->mode)
  {
    case MODE_POLL: {
    } break;
    
    case MODE_SLAVE: {
    } break;
  }
  
  if (self->mode == MODE_POLL) {
    // ... TODO
  } else {
    //printf("[process_pld] init   pos is %03d\n", *pos);
    // определяем тип команды управления
    cmd = B_TO_S( buf[*pos], buf[*pos+1] );
    *pos += 2;
    if (cmd & 0x1) self->cmd_rcvd = CMD_TIMESET;
    else self->cmd_rcvd = CMD_PARAMETERS;
    
    //printf("[decode_head] #1\r\n");
    //printf("[process_pld] next   pos is %03d\n", *pos);
    // Определяем текущий Датасет (с 1 по 6 включит.)
    self->ds = (u8_t)SUB_TO_DS(buf[*pos]);
    // Причем кол-во ds должно быть не больше 1+6=7
    if ( (self->ds < SER_MIN_DS_IDX) || (self->ds > SER_MAX_DS_IDX) ) {
      return -1;
    }
    // Определение страницы
    self->page = (u8_t)B_TO_PG(buf[*pos]);
    if ( self->page > SER_MAX_PAGE_IDX ) {
      return -1;
    }
    *pos += 1;
    //printf("[process_pld] finish pos is %03d\n", *pos);
  }
    
  return 0;
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
static void encode_head(ser_t self)
{
  u8_t  *buf;
  //u32_t *pos = &self->xmit.pos;
  u32_t *size;
  
  assert(self);
  buf = self->xmit.buf;
  size = &self->xmit.size;
  
  // printf("[encode_head] *pos:%02d (#1)\r\n", *pos);
  
  switch (self->mode)
  {
    case MODE_POLL: {
    } break;
    
    case MODE_SLAVE: {
    } break;
  }
  
  if (self->mode == MODE_POLL) {
    if (self->page >= SER_MAX_PAGE_IDX) self->page = SER_MIN_PAGE_IDX;
    else self->page += 1;
    
    if (self->page == SER_MIN_PAGE_IDX) {
      if (self->ds >= SER_MAX_DS_IDX) self->ds = SER_MIN_DS_IDX;
      else self->ds += 1;
    }
    
    S_TO_PB( (buf+*size), self->cmd_xmit );
    *size += 2;
    buf[*size] = DS_TO_B(self->ds) | self->page;
    *size += 1;
    
    printf("[encode_head] ds:%02d, page:%02d, *size:%02d, xmit:%01d\n", 
            self->ds, self->page, *size, self->cmd_xmit);
    
  } else {
    // ... TODO
  }
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
static void process_pld(ser_t self)
{
  u8_t  *buf;
  u32_t *pos;
  
  assert(self);
  buf = self->rcvd.buf;
  pos = &self->rcvd.pos;
  
  switch (self->mode)
  {
    case MODE_POLL: {
    } break;
    
    case MODE_SLAVE: {
    } break;
  }
  
  
  if (self->mode == MODE_POLL) {
    // ... TODO
  } else {
    //printf("[process_pld] #1\r\n");
    //printf("[process_pld] init   pos is %03d\n", *pos);
    // Кладём данные из входного буфера в другой временный буфер.
    // Потом временный буфер надо будет заменить реальными переменными. Эти
    // переменные будут представлять собой поля DataSet-ов с 1 по 6 гололёдов.
    // указатель байта в начальное состояние
    for (u32_t i=0; i<SER_PAGE_SIZE; i++) {
      self->cargvalue2[i] = B_TO_S( buf[*pos], buf[*pos+1] );
      *pos += 2;
    }
    
    //printf("[process_pld] next   pos is %03d\n", *pos);
    // Вызываем функцию обновления полей датасетов
    if (self->carg_fn)
      self->carg_fn( self->pld_api, self->cargvalue2, SER_PAGE_SIZE,
                      self->ds, self->page );
    
    //printf("[process_pld] #2\r\n");
    // Заполняем поля структуры значениями буфера
    // указатель байта в начальное состояние
    // Параметр [0..10]
    for (u32_t i=0; i<SER_NUM_SUBS; i++) {
      //printf("[process_pld] iter   pos is %03d\n", *pos);
      self->subsprm2[i].sl = (s32_t)B_TO_S( buf[*pos], buf[*pos+1] );
      *pos += 2;
      self->subsprm2[i].pul[0] = B_TO_L( buf[*pos+0], buf[*pos+1], buf[*pos+2],
                                       buf[*pos+3] );
      *pos += 4;
      self->subsprm2[i].pul[1] = B_TO_S(buf[*pos], buf[*pos+1]) * 1000U;
      *pos += 2;
    }
    //printf("[process_pld] finish pos is %03d\n", *pos);
    // пробегаемся по всем 11 полям
    if (self->subs_fn)
      self->subs_fn( self->pld_api, self->subsprm2, SER_NUM_SUBS );
  }
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
static void compose_pld(ser_t self)
{
  u8_t  *buf;
  //u32_t *pos = &self->xmit.pos;
  u32_t *size;
  
  assert(self);
  buf = self->xmit.buf;
  size = &self->xmit.size;
  
  switch (self->mode)
  {
    case MODE_POLL: {
      printf("[compose_pld] *size:%02d (#1)\r\n", *size);
      // Вызываем функцию для записи в поля текущ. страницы
      if (self->carg_fn)
        self->carg_fn( self->pld_api, self->cargvalue2, SER_PAGE_SIZE,
                        self->ds, self->page );
      // бежим по текущ. странице
      for (u32_t i=0; i<SER_PAGE_SIZE; i++) {
        S_TO_PB((buf+*size), self->cargvalue2[i]);
        *size += 2;
      }
      // Вызываем функцию для записи в поля подписок
      if (self->subs_fn)
        self->subs_fn( self->pld_api, self->subsprm2, SER_NUM_SUBS );
      // бежим по подпискам
      for (u32_t i=0; i<SER_NUM_SUBS; i++) {
        S_TO_PB(buf+*size, (s16_t)(self->subsprm2[i].sl & 0xffff));
        *size += 2;
        I_TO_PB(buf+*size, self->subsprm2[i].pul[0]);
        *size += 4;
        S_TO_PB(buf+*size, (s16_t)(self->subsprm2[i].pul[1] & 0xffff));
        *size += 2;
      }
      //printf("[compose_pld] *pos:%02d (#3)\r\n", *pos);
    } break;
    
    case MODE_SLAVE: {
      // команда: передача значений
      if (self->cmd_rcvd == CMD_PARAMETERS) {
        if (self->answ_fn)
          self->answ_fn( self->pld_api, self->answ_buf, &self->answ_len );
        if ( !self->answ_len ) return;
        // бежим по всем значениям, записанным 'answ_fn' в 'answ_buf'
        for (u32_t i=0; i < self->answ_len; i++) {
          S_TO_PB(buf+*size, self->answ_buf[i]);
          *size += 2;
        }
      }
      // команда: передача времени
      else if (self->cmd_rcvd == CMD_TIMESET) {
        // берем время
        //IFRS_GET_SYSTEM_TIME(&ulTime, &ulUsec);
        // полезная нагрузка
        // time_t
    /*     self->xmit.buf[2] = (u8_t)((ulTime & 0xff000000) >> 24);
        self->xmit.buf[3] = (u8_t)((ulTime & 0x00ff0000) >> 16);
        self->xmit.buf[4] = (u8_t)((ulTime & 0x0000ff00) >> 8);
        self->xmit.buf[5] = (u8_t)(ulTime & 0x00ff);
        // usec
        self->xmit.buf[6] = (u8_t)((ulUsec & 0x0000ff00) >> 8);
        self->xmit.buf[7] = (u8_t)(ulUsec & 0x00ff); */
        // buf[*pos++] = 0;
        // buf[*pos++] = 0;
        // buf[*pos++] = 0;
        // buf[*pos++] = 0;
        // buf[*pos++] = 0;
      }
    } break;
  }
  
  if (self->mode == MODE_POLL) {
    printf("[compose_pld] *size:%02d (#1)\r\n", *size);
    // Вызываем функцию для записи в поля датасетов
    if (self->carg_fn)
      self->carg_fn( self->pld_api, self->cargvalue2, SER_PAGE_SIZE,
                      self->ds, self->page );
    
    // бежим по странице
    for (u32_t i=0; i<SER_PAGE_SIZE; i++) {
      S_TO_PB((buf+*size), self->cargvalue2[i]);
      *size += 2;
    }
    
    //printf("[compose_pld] *pos:%02d (#2)\r\n", *pos);
    
    // Вызываем функцию для записи в поля подписок
    if (self->subs_fn)
      self->subs_fn( self->pld_api, self->subsprm2, SER_NUM_SUBS );
    
    // бежим по подпискам
    for (u32_t i=0; i<SER_NUM_SUBS; i++) {
      S_TO_PB((buf+*size), (s16_t)(self->subsprm2[i].sl & 0xffff));
      //S_TO_PB(&buf[*pos], (s16_t)(self->subsprm2[i].sl & 0xffff));
      *size += 2;
      I_TO_PB(buf+*size, self->subsprm2[i].pul[0]);
      *size += 4;
      S_TO_PB(buf+*size, (s16_t)(self->subsprm2[i].pul[1] & 0xffff));
      *size += 2;
    }
    //printf("[compose_pld] *pos:%02d (#3)\r\n", *pos);
  } else {

  }
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
buf_rcvd_t ser_get_buf_rcvd(ser_t self)
{
  assert(self);
  return &self->rcvd;
}

/**
  * @brief  ?
  * @param  self - ?
  * @retval ?
  */
buf_xmit_t ser_get_buf_xmit(ser_t self)
{
  assert(self);
  return &self->xmit;
}
