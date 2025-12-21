/**
* @file ser.h
* @author Ilia Proniashin, msg@proglyk.ru
* @date 30-September-2025
*/

#ifndef SER2MMS_SER_H
#define SER2MMS_SER_H

#include "ser2mms_conf.h"
#include "types.h"
#include "port_types.h"

// Allocation
#define SER_USE_STATIC                  (0) //S2M_USE_STATIC

// Debug output
#define SER_DEBUG                       S2M_DEBUG

//#define MB_SER_PDU_SIZE_MAX (16) // Maximum size of a Modbus RTU frame.
#define REGS_NUM                        (125)

// Table size (number) of registers available for read and write (R/W).
#define MB_INPUT_SIZE                   REGS_NUM
#define MB_HOLD_SIZE                    REGS_NUM

// Buffer size
#define BUFSIZE                         (2*REGS_NUM)

// Short names
#define MB_RCV_BUF(S)                   (GET_RCVD(S)->buf)
#define MB_RCV_POS(S)                   (GET_RCVD(S)->pos)
#define MB_RCV_SIZE(S)                  (GET_RCVD(S)->size)
#define MB_XMT_BUF(S)                   (GET_XMIT(S)->buf)
#define MB_XMT_POS(S)                   (GET_XMIT(S)->pos)
#define MB_XMT_SIZE(S)                  (GET_XMIT(S)->size)

// Short names for func declarations
//#define buf_t ser_buf_t // TODO too generic

// Short names for func declarations
#define GET_RCVD(S)                     ser_get_buf_rcvd(S)
#define GET_XMIT(S)                     ser_get_buf_xmit(S)

// Struct declaration
struct ser_buf_s {
  u8_t *p;
  u32_t pos;
  u32_t size;
};

struct buf_rcvd_s {
  u8_t buf[BUFSIZE];
  u32_t pos;
  u32_t size;
};

struct buf_xmit_s {
  u8_t buf[BUFSIZE+10];
  u32_t pos;
  u32_t size;
};

// Type declaration
typedef struct buf_rcvd_s *buf_rcvd_t;
typedef struct buf_xmit_s *buf_xmit_t;
typedef struct ser_buf_s *ser_buf_t;
typedef struct ser_s *ser_t;

/**
* @brief Creates a new instance of 'struct ser_s' object
* @param mode Operating mode (MODE_POLL or MODE_SLAVE)
* @param carg_cb Callback function for updating parameters/data
* @param subs_cb Callback function for updating subscription parameters
* @param answ_cb Callback function for generating response
* @param pld_api Pointer to payload API context
* @retval Returns pointer to this instance or NULL if failed to
* allocate memory or object is already in use (static allocation)
*/
ser_t ser_new(topmode_t mode, void *pld_api);

/**
* @brief Destroys 'struct ser_s' object and releases resources
* @param self Pointer to this instance
*/
void ser_destroy(ser_t self);

/**
* @brief Sets command type for next transmission
* @param self Pointer to this instance
* @param value Command value (0 - CMD_PARAMETERS, non-zero - CMD_TIMESET)
*/
void ser_set_cmd(ser_t self, u32_t value);

/**
* @brief Parses received incoming message and processes payload
* @param self Pointer to this instance
* @retval 0 on success
* @retval -1 on error (invalid size, dataset or page index)
*/
s32_t ser_in_parse(ser_t self);

/**
* @brief Builds outgoing message with header and payload for
* transmission
* @param self Pointer to this instance
*/
void ser_out_build(ser_t self);

/**
* @brief Returns pointer to receive buffer structure
* @param self Pointer to this instance
* @retval buf_rcvd_t Pointer to receive buffer
*/
buf_rcvd_t ser_get_buf_rcvd(ser_t self);

/**
* @brief Returns pointer to transmit buffer structure
* @param self Pointer to this instance
* @retval buf_xmit_t Pointer to transmit buffer
*/
buf_xmit_t ser_get_buf_xmit(ser_t self);

#endif
