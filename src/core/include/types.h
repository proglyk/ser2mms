/**
  * @file   types.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   22-September-2025
  */

#ifndef  SER2MMS_TYPES_H
#define  SER2MMS_TYPES_H

#include "port_types.h"

typedef struct cargvalue_s cargvalue_t;
typedef struct outdata_s outdata_t;

typedef enum {
	CMD_PARAMETERS,	CMD_TIMESET
} cmd_t;

typedef enum {
	MODE_SLAVE, MODE_POLL
} topmode_t;

typedef struct {
	s32_t sl;
	u32_t pul[2];
} prm_t;

struct cargvalue_s {
	u16_t usParam0;
	u16_t usParam1;
	u16_t usParam2;
};

struct outdata_s {
	u16_t usIz;
	u16_t usRlz;
	u16_t usCirc;
};

typedef void (*carg_fn_t)(void *, u16_t *, u32_t, u8_t, u8_t);
typedef void (*subs_fn_t)(void *, prm_t *, u32_t);
typedef void (*answ_fn_t)(void *, u16_t *, u32_t *);

#endif