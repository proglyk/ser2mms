/**
 * @file ser_types.h
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 22-September-2025
 * 
 * Serial protocol type definitions.
 * Defines data types, structures and macros used in the serial protocol.
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

/** Response mode: parameters / system time transmission. */
typedef enum {
  CMD_PARAMETERS, CMD_TIMESET
} ser_cmd_t;

/** Subscription parameter sub. */
typedef struct {
  s32_t sl;
  u32_t pul[2];
} prm_t;

/** Page parameters. */
typedef struct {
  u16_t usParam0;
  u16_t usParam1;
  u16_t usParam2;
} cargvalue_t;

/** Response parameters. */
typedef struct {
  u16_t usIz;
  u16_t usRlz;
  u16_t usCirc;
} outdata_t;

#endif
