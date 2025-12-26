/**
 * @file ser.c
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 30-September-2025
 * 
 * Serial protocol implementation.
 */

#include "ser.h"
#include "alloc.h"
#include "byteops.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/**
 * Internal serial protocol handler structure.
 */
struct ser_s {
  ser_cmd_t cmd_rcvd;                  // Received command: 0 - normal mode,
                                       // 1 - time transmission
  struct buf_rcvd_s rcvd;              // Receive buffer
  ser_cmd_t cmd_xmit;                  // Command for transmission
  struct buf_xmit_s xmit;              // Transmit buffer
  ser_mode_t mode;                     // Operation mode
  u8_t ds;                             // Dataset index
  u8_t page;                           // Page number
  u16_t cargvalue2[SER_PAGE_SIZE];     // Command argument values
#if (!S2M_REDUCED)
  prm_t subsprm2[SER_NUM_SUBS];        // Subscription parameters
#endif
  u16_t answ_buf[SER_ANSW_SIZE];       // Answer buffer
  u32_t answ_len;                      // Answer length
  void *pld_api;                       // Pointer to payload API
};

STATIC_DECLARE(SER, struct ser_s);

// Private function declarations
extern void __WEAK ser2mms_set_time(uint32_t *, uint32_t *);
extern void __WEAK ser2mms_read_carg(void *, u16_t *, u32_t, u8_t, u8_t);
extern void __WEAK ser2mms_read_subs(void *, prm_t *, u32_t);
extern void __WEAK ser2mms_write_answer(void *, u16_t *, u32_t *);

static s32_t decode_head(ser_t);
static void encode_head(ser_t);
static void process_pld(ser_t);
static void compose_pld(ser_t);

// Public interface function definitions

// Basic functions

/**
 * Constructor.
 */
ser_t ser_new(ser_mode_t mode, void *pld_api)
{
  ALLOC(SER, struct ser_s, self, return NULL);
  self->mode = mode;
  self->ds = SER_MAX_DS_IDX;
  self->page = SER_MAX_PAGE_IDX;
  self->pld_api = pld_api;
  return self;
}

/**
 * Destructor.
 */
void ser_destroy(ser_t self)
{
  assert(self);
  FREE(SER, self);
}

// Parse incoming, build outgoing packets

/**
 * Parse incoming message.
 */
s32_t ser_in_parse(ser_t self)
{
  u32_t size;
  assert(self);

  size = (self->mode == MODE_SLAVE) ? IN_MSG_SIZE_SLAVE : IN_MSG_SIZE_POLL;
  if (self->rcvd.size != size) {
    printf("[ser_in_parse] Size %d doesn't match expected (%d)\n", self->rcvd.size, size);
    return -1;
  }

  // Parse header
  if (decode_head(self) < 0) {
    printf("[ser_in_parse] Failed to decode header\n");
    return -1;
  }

  // Parse payload
  process_pld(self);
  return 0;
}

/**
 * Build outgoing message.
 */
void ser_out_build(ser_t self)
{
  assert(self);

  // Build header
  encode_head(self);

  // Build payload
  compose_pld(self);
}

// Helper functions

/**
 * Set command type.
 */
void ser_set_cmd(ser_t self, u32_t value)
{
  assert(self);
  self->cmd_xmit = value ? CMD_TIMESET : CMD_PARAMETERS;
}

/**
 * Get pointer to receive buffer.
 */
buf_rcvd_t ser_get_buf_rcvd(ser_t self)
{
  assert(self);
  return &self->rcvd;
}

/**
 * Get pointer to transmit buffer.
 */
buf_xmit_t ser_get_buf_xmit(ser_t self)
{
  assert(self);
  return &self->xmit;
}

// Private function definitions

/**
 * Parse message header.
 * Extracts command, dataset and page information from message header.
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
      // Determine acknowledgment type of control command received in response
      cmd = B_TO_S(buf[*pos], buf[*pos+1]);
      *pos += 2;
      self->cmd_rcvd = (cmd & 0x1) ? (CMD_TIMESET) : (CMD_PARAMETERS);
    } break;

    case MODE_SLAVE:
    {
      // Determine control command type
      cmd = B_TO_S(buf[*pos], buf[*pos+1]);
      *pos += 2;
      self->cmd_rcvd = (cmd & 0x1) ? (CMD_TIMESET) : (CMD_PARAMETERS);

      // Determine current dataset (from 1 to 6 inclusive)
      self->ds = (u8_t)SUB_TO_DS(buf[*pos]);
      // Dataset number must not exceed 1+6=7
      if ((self->ds < SER_MIN_DS_IDX) || (self->ds > SER_MAX_DS_IDX)) {
        return -1;
      }

      // Determine page
      self->page = (u8_t)B_TO_PG(buf[*pos]);
      if (self->page > SER_MAX_PAGE_IDX) return -1;
      *pos += 1;
    } break;
  }

  return 0;
}

/**
 * Process received message payload.
 */
static void process_pld(ser_t self)
{
  u8_t *buf;
  u32_t *pos;

  assert(self);
  buf = self->rcvd.buf;
  pos = &self->rcvd.pos;

  switch (self->mode)
  {
    case MODE_POLL:
    {
      // Command: parameter transmission
      if (self->cmd_rcvd == CMD_PARAMETERS) {
        printf("[process_pld] Setpoint Iz: %02d\n", buf[4]);
        printf("[process_pld] Setpoint Rlz: %02d\n", buf[6]);
        printf("[process_pld] Setpoint Circ: %02d\n", buf[8]);
      }
      // Command: time transmission
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
      // Place data from input buffer into temporary buffer.
      // Later the temporary buffer should be replaced with real variables. These
      // variables will represent DataSet fields from 1 to 6 for ice loads.
      for (u32_t i=0; i<SER_PAGE_SIZE; i++) {
        self->cargvalue2[i] = B_TO_S(buf[*pos], buf[*pos+1]);
        *pos += 2;
      }

      // Call function to update dataset fields
      ser2mms_read_carg(self->pld_api, self->cargvalue2, SER_PAGE_SIZE,
                        self->ds, self->page);

      // Fill structure fields with values from buffer
      // Parameter [0..10]
#if (!S2M_REDUCED)
      for (u32_t i=0; i<SER_NUM_SUBS; i++) {
        self->subsprm2[i].sl = (s32_t)B_TO_S(buf[*pos], buf[*pos+1]);
        *pos += 2;
        self->subsprm2[i].pul[0] = B_TO_L(buf[*pos+0], buf[*pos+1], buf[*pos+2],
                                          buf[*pos+3]);
        *pos += 4;
        self->subsprm2[i].pul[1] = B_TO_S(buf[*pos], buf[*pos+1]) * 1000U;
        *pos += 2;
      }

      // Iterate through all 11 fields
      ser2mms_read_subs(self->pld_api, self->subsprm2, SER_NUM_SUBS);
#endif
    } break;
  }
}

/**
 * Encode message header.
 * Forms header with command, dataset and page information.
 */
static void encode_head(ser_t self)
{
  u8_t *buf;
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

      // In POLL mode use cmd_xmit field value as Cmd,
      // which can be changed anytime externally via
      // 'ser_set_cmd' method
      S_TO_PB((buf+*size), self->cmd_xmit);
      *size += 2;

      buf[*size] = DS_TO_B(self->ds) | self->page;
      *size += 1;
    } break;

    case MODE_SLAVE:
    {
      // In SLAVE mode use cmd_rcvd field value as Cmd,
      // which was received in incoming packet. It's set on master
      // side and we can't change it, only duplicate in response, using
      // it as successful reception acknowledgment
      S_TO_PB((buf+*size), self->cmd_rcvd);
      *size += 2;
    } break;
  }
}

/**
 * Build payload for transmission.
 */
static void compose_pld(ser_t self)
{
  u8_t *buf;
  u32_t *size;
  uint32_t ts[2];

  assert(self);
  buf = self->xmit.buf;
  size = &self->xmit.size;

  switch (self->mode)
  {
    case MODE_POLL:
    {
      // Call function to write current page fields
      ser2mms_read_carg(self->pld_api, self->cargvalue2, SER_PAGE_SIZE,
                        self->ds, self->page);

      // Iterate through current page
      for (u32_t i=0; i<SER_PAGE_SIZE; i++) {
        S_TO_PB(buf+*size, self->cargvalue2[i]);
        *size += 2;
      }

#if (!S2M_REDUCED)
      // Call function to write subscription fields
      ser2mms_read_subs(self->pld_api, self->subsprm2, SER_NUM_SUBS);

      // Iterate through subscriptions
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
      // Command: parameter transmission
      if (self->cmd_rcvd == CMD_PARAMETERS)
      {
        // Call functor to get values into 'answ_buf'
        ser2mms_write_answer(self->pld_api, self->answ_buf, &self->answ_len);

        // Validate array by its size 'answ_len'
        if (!self->answ_len) return;

        // Iterate through all 'answ_buf' values
        for (u32_t i=0; i < self->answ_len; i++) {
          S_TO_PB(buf+*size, self->answ_buf[i]);
          *size += 2;
        }
      }
      // Command: time transmission
      else if (self->cmd_rcvd == CMD_TIMESET)
      {
        // Set time from external source
        ser2mms_set_time(&ts[0], &ts[1]);

        // Iterate through 'ts' fields
        I_TO_PB(buf+*size, ts[0]);
        *size += 4;
        S_TO_PB(buf+*size, (ts[1] & 0x0000ffff));
        *size += 2;
      }
    } break;
  }
}
