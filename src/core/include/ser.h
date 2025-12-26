/**
 * @file ser.h
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 30-September-2025
 * 
 * Serial protocol interface.
 * Provides functions for working with serial data exchange protocol,
 * including parsing incoming messages and building outgoing packets.
 */

#ifndef SER2MMS_SER_H
#define SER2MMS_SER_H

#include "ser2mms_conf.h"
#include "ser_types.h"
#include "port_types.h"

/** Use static allocation. */
#define SER_USE_STATIC (0) //S2M_USE_STATIC

/** Debug output. */
#define SER_DEBUG S2M_DEBUG

/** Buffer size. */
#define BUFSIZE (2*125)

/** 'ser' module settings. */
#if (S2M_REDUCED)
#define IN_MSG_SIZE_SLAVE (12) //test
#else
#define SER_NUM_SUBS (11)
#define IN_MSG_SIZE_SLAVE (100) //production
#endif

/** 'ser' module settings. */
#define SER_MIN_DS_IDX (1)
#define SER_MAX_DS_IDX (6)
#define SER_MIN_PAGE_IDX (0)
#define SER_MAX_PAGE_IDX (3)
#define SER_PAGE_SIZE (3)
#define IN_MSG_SIZE_POLL (11)
#define SER_ANSW_SIZE (3)

/** Shorthand for calling getter for receive buffer pointer. */
#define GET_RCVD(S) ser_get_buf_rcvd(S)

/** Shorthand for calling getter for transmit buffer pointer. */
#define GET_XMIT(S) ser_get_buf_xmit(S)

/**
 * Serial protocol buffer structure.
 */
struct ser_buf_s {
  u8_t *p;      // Pointer to data
  u32_t pos;    // Current position
  u32_t size;   // Data size
};

/**
 * Receive buffer structure.
 */
struct buf_rcvd_s {
  u8_t buf[BUFSIZE];  // Data buffer
  u32_t pos;          // Current position
  u32_t size;         // Data size
};

/**
 * Transmit buffer structure.
 */
struct buf_xmit_s {
  u8_t buf[BUFSIZE+10];  // Data buffer
  u32_t pos;             // Current position
  u32_t size;            // Data size
};

/** Pointer type to receive buffer. */
typedef struct buf_rcvd_s *buf_rcvd_t;

/** Pointer type to transmit buffer. */
typedef struct buf_xmit_s *buf_xmit_t;

/** Pointer type to serial protocol buffer. */
typedef struct ser_buf_s *ser_buf_t;

/** Pointer type to 'ser' object. */
typedef struct ser_s *ser_t;

// Public interface function declarations

// Basic functions

/**
 * Serial protocol object constructor.
 * Creates a new protocol handler instance with specified operation mode.
 * 
 * @param mode operation mode (MODE_POLL or MODE_SLAVE)
 * @param pld_api pointer to payload API context
 * @return pointer to created instance or NULL on allocation error
 */
ser_t ser_new(ser_mode_t mode, void *pld_api);

/**
 * Serial protocol object destructor.
 * Destroys protocol handler instance and frees resources.
 * 
 * @param self pointer to instance
 */
void ser_destroy(ser_t self);

// Parse incoming, build outgoing packets

/**
 * Parse incoming message.
 * Analyzes received message, extracts header and processes payload.
 * 
 * @param self pointer to instance
 * @return 0 on success, -1 on error (invalid size, dataset or page index)
 */
s32_t ser_in_parse(ser_t self);

/**
 * Build outgoing message.
 * Assembles outgoing message with header and payload for transmission.
 * 
 * @param self pointer to instance
 */
void ser_out_build(ser_t self);

// Helper functions

/**
 * Set command type for next transmission.
 * Defines the command type to be sent in the next outgoing message.
 * 
 * @param self pointer to instance
 * @param value command value (0 - CMD_PARAMETERS, non-zero - CMD_TIMESET)
 */
void ser_set_cmd(ser_t self, u32_t value);

/**
 * Get pointer to receive buffer structure.
 * Returns pointer to buffer for placing received data.
 * 
 * @param self pointer to instance
 * @return pointer to receive buffer
 */
buf_rcvd_t ser_get_buf_rcvd(ser_t self);

/**
 * Get pointer to transmit buffer structure.
 * Returns pointer to buffer for placing data to transmit.
 * 
 * @param self pointer to instance
 * @return pointer to transmit buffer
 */
buf_xmit_t ser_get_buf_xmit(ser_t self);

#endif
