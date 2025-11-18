/**
  * @file   mms_if.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   09-October-2025
  */

#ifndef SER2MMS_MMS_IF_H
#define SER2MMS_MMS_IF_H

#include "ser2mms_conf.h"
#include "port_types.h"

#if (LIBIEC_EXIST)

#include "libiec61850/api/ied_server_api.h"
#include "libiec61850/api/model.h"
#include "libiec61850/mms/mms_types.h"

#else //LIBIEC_EXIST
  
struct null_s {
  int null;
};
typedef struct null_s MmsValue;
typedef struct null_s DataAttribute;

#endif //LIBIEC_EXIST

// Decorator for set attribute func
#define MMS_SET_ATTR(type, S, PX, VAL) mms_if_set_attr_##type(S->ied, PX, VAL)

// Func prototypes
bool mms_if_set_attr_s32( void *, DataAttribute*, s32_t );
bool mms_if_set_attr_f32( void *, DataAttribute*, f32_t );
bool mms_if_set_attr_t( void *, DataAttribute*, u32_t * );
bool mms_if_set_attr_q( void *, DataAttribute*, bool );

#endif