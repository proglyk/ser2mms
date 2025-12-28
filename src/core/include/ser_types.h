/**
* @file ser_types.h
* @author Ilia Proniashin, msg@proglyk.ru
* @date 22-September-2025
*
* Type definitions for serial protocol handler.
*/
#ifndef SER2MMS_TYPES_H
#define SER2MMS_TYPES_H

#include "port_types.h"

/** Extract dataset index from byte. */
#define SUB_TO_DS(a) (((a) >> 4) & 0x0f)

/** Encode dataset index into byte. */
#define DS_TO_B(a) (((a) << 4) & 0xf0)

/** Combine two bytes into 16-bit subscription value. */
#define BTOSUB(a,b) ((((a) << 8) & 0x0000ff00) | \
                     (((b) << 0) & 0x000000ff))

/** Extract lower 4 bits of byte as page number. */
#define B_TO_PG(a) ((a) & 0x0f)

/** Operation mode: slave/master. */
typedef enum {
  MODE_SLAVE, MODE_POLL
} ser_mode_t;

/** Response mode: parameters transfer / system time. */
typedef enum {
  CMD_PARAMETERS, CMD_TIMESET
} ser_cmd_t;

/** Subscription parameter. */
typedef struct {
  s16_t mag;    // Magnitude value
  u32_t t[2];   // Timestamp
} sub_prm_t;

/** Page parameter. */
typedef struct {
  s16_t mag;    // Magnitude value
} page_prm_t;

/** Answer parameter. */
typedef struct {
  s16_t mag;    // Magnitude value
} answ_prm_t;

#endif
