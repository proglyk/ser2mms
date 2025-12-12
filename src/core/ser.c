/**
* @file ser.c
* @author Ilia Proniashin, msg@proglyk.ru
* @date 30-September-2025
*/

#include "ser.h"
#include "alloc.h"
#include "macro.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern void __WEAK ser2mms_get_time(uint32_t *epoch, uint32_t *usec);

static s32_t decode_head(ser_t);
static void encode_head(ser_t);
static void process_pld(ser_t);
static void compose_pld(ser_t);

struct ser_s {
  // Received
  cmd_t cmd_rcvd;  // Control: 0 – normal mode, 1 – time transfer
  struct buf_rcvd_s rcvd;
  // For transmit
  cmd_t cmd_xmit;
  struct buf_xmit_s xmit;
  // General
  topmode_t mode;
  u8_t ds;
  u8_t page;
  u16_t cargvalue2[SER_PAGE_SIZE];
#if (!S2M_REDUCED)
  prm_t subsprm2[SER_NUM_SUBS];
#endif
  u16_t answ_buf[SER_ANSW_SIZE];
  u32_t answ_len;
  carg_fn_t carg_fn;
  subs_fn_t subs_fn;
  answ_fn_t answ_fn;
  void *pld_api;
};
STATIC_DECLARE(SER, struct ser_s);

// ============================= Public functions ==============================

/**
* @brief Creates a new instance of serial protocol handler
*/
ser_t ser_new(topmode_t mode, carg_fn_t carg_cb, subs_fn_t subs_cb,
              answ_fn_t answ_cb, void *pld_api)
{
  ALLOC(SER, struct ser_s, self, return NULL);
  self->mode = mode;
  self->ds = SER_MAX_DS_IDX;
  self->page = SER_MAX_PAGE_IDX;
  self->carg_fn = carg_cb;
  self->subs_fn = subs_cb;
  self->answ_fn = answ_cb;
  self->pld_api = pld_api;
  return self;
}

/**
* @brief Destroys serial protocol handler instance and releases resources
*/
void ser_destroy(ser_t self)
{
  assert(self);
  FREE(SER, self);
}

/**
* @brief Parses received incoming message and processes payload
*/
s32_t ser_in_parse(ser_t self)
{
  u32_t size;
  assert(self);
  
  size = (self->mode == MODE_SLAVE) ? IN_MSG_SIZE_SLAVE : IN_MSG_SIZE_POLL;
  if (self->rcvd.size != size) return -1;
  
  // parse header
  if (decode_head(self) < 0) return -1;
  // parse payload
  process_pld(self);
  return 0;
}

/**
* @brief Builds outgoing message with header and payload for transmission
*/
void ser_out_build(ser_t self)
{
  assert(self);
  
  // build header
  encode_head(self);
  // build payload
  compose_pld(self);
}

/**
* @brief Sets command type for next transmission
*/
void ser_set_cmd(ser_t self, u32_t value)
{
  assert(self);
  self->cmd_xmit = value ? CMD_TIMESET : CMD_PARAMETERS;
}

/**
* @brief Returns pointer to receive buffer structure
*/
buf_rcvd_t ser_get_buf_rcvd(ser_t self)
{
  assert(self);
  return &self->rcvd;
}

/**
* @brief Returns pointer to transmit buffer structure
*/
buf_xmit_t ser_get_buf_xmit(ser_t self)
{
  assert(self);
  return &self->xmit;
}

// ============================= Static functions ==============================

/**
* @brief Parses message header and extracts command, dataset and
* page information
*/
static s32_t decode_head(ser_t self)
{
  u8_t *buf;
  u32_t *pos;
  u16_t cmd;
  assert(self);
  
  buf = self->rcvd.buf;
  pos = &self->rcvd.pos;
  
  switch (self->mode)
  {
    case MODE_POLL:
    {
      // determine type of control command acknowledgment received in response
      cmd = B_TO_S( buf[*pos], buf[*pos+1] );
      *pos += 2;
      self->cmd_rcvd = (cmd & 0x1) ? (CMD_TIMESET) : (CMD_PARAMETERS);
    } break;
    case MODE_SLAVE:
    {
      // determine type of control command
      cmd = B_TO_S( buf[*pos], buf[*pos+1] );
      *pos += 2;
      self->cmd_rcvd = (cmd & 0x1) ? (CMD_TIMESET) : (CMD_PARAMETERS);
      // Determine current Dataset (from 1 to 6 inclusive)
      self->ds = (u8_t)SUB_TO_DS(buf[*pos]);
      // Moreover, the number of ds should not be greater than 1+6=7
      if ( (self->ds < SER_MIN_DS_IDX) || (self->ds > SER_MAX_DS_IDX) ) {
        return -1;
      }
      // Determine page
      self->page = (u8_t)B_TO_PG(buf[*pos]);
      if ( self->page > SER_MAX_PAGE_IDX ) return -1;
      *pos += 1;
    } break;
  }
  return 0;
}

/**
* @brief Processes payload data from received message
*/
static void process_pld(ser_t self)
{
  u8_t *buf;
  u32_t *pos;
  u32_t *size;
  assert(self);
  
  buf = self->rcvd.buf;
  pos = &self->rcvd.pos;
  size = &self->xmit.size;
  
  switch (self->mode)
  {
    case MODE_POLL:
    {
      // command: parameter transfer
      if (self->cmd_rcvd == CMD_PARAMETERS) {
        printf("[process_pld] Setpoint Iz is %02d\n", buf[4]);
        printf("[process_pld] Setpoint Rlz is %02d\n", buf[6]);
        printf("[process_pld] Setpoint Circ is %02d\n", buf[8]);
      }
      // command: time transfer
      else if (self->cmd_rcvd == CMD_TIMESET) {
        u32_t ul = B_TO_L(buf[*pos], buf[*pos+1], buf[*pos+2], buf[*pos+3]);
        *pos += 4;
        printf("[process_pld] Epoch #1: %010d\n", ul);
        u16_t us = B_TO_S(buf[*pos], buf[*pos+1]);
        *pos += 2;
        printf("[process_pld] Usec #1: %d\n", us);
      }
    } break;
    case MODE_SLAVE:
    {
      // Put data from input buffer into another temporary buffer.
      // Later, the temporary buffer will need to be replaced with real variables. These
      // variables will represent the DataSet fields from 1 to 6 for ice loads.
      // byte pointer to initial state
      for (u32_t i=0; i<SER_PAGE_SIZE; i++) {
        self->cargvalue2[i] = B_TO_S( buf[*pos], buf[*pos+1] );
        *pos += 2;
      }
      // Call function to update dataset fields
      if (self->carg_fn)
        self->carg_fn( self->pld_api, self->cargvalue2, SER_PAGE_SIZE,
                       self->ds, self->page );
      // Fill structure fields with buffer values
      // byte pointer to initial state
      // Parameter [0..10]
#if (!S2M_REDUCED)
      for (u32_t i=0; i<SER_NUM_SUBS; i++) {
        self->subsprm2[i].sl = (s32_t)B_TO_S( buf[*pos], buf[*pos+1] );
        *pos += 2;
        self->subsprm2[i].pul[0] = B_TO_L( buf[*pos+0], buf[*pos+1], buf[*pos+2],
                                            buf[*pos+3] );
        *pos += 4;
        self->subsprm2[i].pul[1] = B_TO_S(buf[*pos], buf[*pos+1]) * 1000U;
        *pos += 2;
      }
      // iterate through all 11 fields
      if (self->subs_fn && (SER_NUM_SUBS>0)) {
        self->subs_fn( self->pld_api, self->subsprm2, SER_NUM_SUBS );
      }
#endif
    } break;
  }
}

/**
* @brief Encodes message header with command, dataset and page information
*/
static void encode_head(ser_t self)
{
  u8_t *buf;
  //u32_t *pos = &self->xmit.pos;
  u32_t *size;
  assert(self);
  
  buf = self->xmit.buf;
  size = &self->xmit.size;
  
  switch (self->mode)
  {
    case MODE_POLL:
    {
      if (self->page >= SER_MAX_PAGE_IDX) self->page = SER_MIN_PAGE_IDX;
      else self->page += 1;
      if (self->page == SER_MIN_PAGE_IDX) {
        if (self->ds >= SER_MAX_DS_IDX) self->ds = SER_MIN_DS_IDX;
        else self->ds += 1;
      }
      // in POLL mode, use the cmd_xmit field value as Cmd,
      // which can be edited at any time from outside via the
      // 'ser_set_cmd' method.
      S_TO_PB( (buf+*size), self->cmd_xmit );
      *size += 2;
      buf[*size] = DS_TO_B(self->ds) | self->page;
      *size += 1;
    } break;
    case MODE_SLAVE:
    {
      // in SLAVE mode, use the cmd_rcvd field value as Cmd,
      // which was received in the incoming packet. It is set on the master side and
      // we cannot change it, but only duplicate it in response using it as
      // acknowledgment that reception was successful.
      S_TO_PB( (buf+*size), self->cmd_rcvd );
      *size += 2;
    } break;
  }
}

/**
* @brief Builds payload data for message transmission
*/
static void compose_pld(ser_t self)
{
  u8_t *buf;
  //u32_t *pos = &self->xmit.pos;
  u32_t *size;
  uint32_t ts[2];
  assert(self);
  
  buf = self->xmit.buf;
  size = &self->xmit.size;
  
  switch (self->mode)
  {
    case MODE_POLL:
    {
      // Call function to write to current page fields
      if (self->carg_fn)
        self->carg_fn( self->pld_api, self->cargvalue2, SER_PAGE_SIZE,
                       self->ds, self->page );
      // iterate through current page
      for (u32_t i=0; i<SER_PAGE_SIZE; i++) {
        S_TO_PB(buf+*size, self->cargvalue2[i]);
        *size += 2;
      }
#if (!S2M_REDUCED)
      // Call function to write to subscription fields
      if (self->subs_fn) {
        self->subs_fn( self->pld_api, self->subsprm2, SER_NUM_SUBS );
      }
      // iterate through subscriptions
      for (u32_t i=0; i<SER_NUM_SUBS; i++) {
        S_TO_PB(buf+*size, (s16_t)(self->subsprm2[i].sl & 0xffff));
        *size += 2;
        I_TO_PB(buf+*size, self->subsprm2[i].pul[0]);
        *size += 4;
        S_TO_PB(buf+*size, (s16_t)(self->subsprm2[i].pul[1] & 0xffff));
        *size += 2;
      }
#endif
    } break;
    case MODE_SLAVE:
    {
      // command: parameter transfer
      if (self->cmd_rcvd == CMD_PARAMETERS) {
        // call functor and get values into 'answ_buf'
        if (self->answ_fn)
          self->answ_fn( self->pld_api, self->answ_buf, &self->answ_len );
        // check array validity by its size 'answ_len'
        if ( !self->answ_len ) return;
        // iterate through all 'answ_buf' values
        for (u32_t i=0; i < self->answ_len; i++) {
          S_TO_PB(buf+*size, self->answ_buf[i]);
          *size += 2;
        }
      }
      // command: time transfer
      else if (self->cmd_rcvd == CMD_TIMESET) {
        // get time
        ser2mms_get_time(&ts[0], &ts[1]);
        // iterate through 'ts' fields
        I_TO_PB(buf+*size, ts[0]);
        *size += 4;
        S_TO_PB(buf+*size, (ts[1] & 0x0000ffff));
        *size += 2;
      }
    } break;
  }
}
