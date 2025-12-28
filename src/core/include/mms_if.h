/**
* @file mms_if.h
* @author Ilia Proniashin, msg@proglyk.ru
* @date 09-October-2025
*
* Interface for working with MMS attributes.
* Provides functions for setting attribute values of various types
* in IEC 61850 data model via MMS (Manufacturing Message Specification).
*/

#ifndef SER2MMS_MMS_IF_H
#define SER2MMS_MMS_IF_H

#include "ser2mms_conf.h"
#include "port_types.h"

#if (S2M_USE_LIBIEC)
#if (S2M_USE_OLD_LIBIEC_API)
#include "libiec61850/api/ied_server_api.h"
#else
#include "iec61850_server.h" // TODO "make version switch"
#endif
#endif

// Functional macro declarations

/**
* Set single attribute.
* Shorthand for calling target attribute value setting function
* with type casting capability for s32, f32, t, q in 'type' field
*/
#define MMS_SET_ATTR(type, PV, PX, VAL) mms_if_set_attr_##type(PV, PX, VAL)

/**
* Set attribute group.
* Sets three attributes of one parameter at once: magnitude value,
* timestamp and quality flag.
*
* @param attr_t   attribute value type (s32, f32)
* @param LN       logical node name
* @param param    data parameter name
* @param mag_suf  suffix for magnitude field (mag_f or mag_i)
* @param c_t C    language type for value casting
* @param mag_val  parameter magnitude value
* @param t_val    pointer to timestamp or NULL
* @param q_val    data quality flag
*/
#define MMS_SET_ATTRS(attr_t, LN, param, mag_suf, c_t, mag_val, t_val, q_val) \
  do { \
    MMS_SET_ATTR(attr_t, ied, IEDMODEL_UPG_##LN##_##param##_##mag_suf, (c_t)mag_val); \
    MMS_SET_ATTR(t, ied, IEDMODEL_UPG_##LN##_##param##_t, t_val); \
    MMS_SET_ATTR(q, ied, IEDMODEL_UPG_##LN##_##param##_q, q_val); \
  } while(0)

/** Wrapper for setting attribute group of one parameter with f32 type in mag. */
#define MMS_SET_ATTRS_F32(LN, param, mag_val, t_val, q_val) \
  MMS_SET_ATTRS(f32, LN, param, mag_f, const f32_t, mag_val, t_val, q_val)

/** Wrapper for setting attribute group of one parameter with int32 type in mag. */
#define MMS_SET_ATTRS_S32(LN, param, mag_val, t_val, q_val) \
  MMS_SET_ATTRS(s32, LN, param, mag_i, const s32_t, mag_val, t_val, q_val)

/** Set attributes for parameter group with common float32 type in mag. */
#define MMS_SET_PARAMS_F32(LN, p1, p2, p3, mag_buf) \
  do { \
    MMS_SET_ATTRS_F32(LN, p1, mag_buf[0].mag, NULL, true); \
    MMS_SET_ATTRS_F32(LN, p2, mag_buf[1].mag, NULL, true); \
    MMS_SET_ATTRS_F32(LN, p3, mag_buf[2].mag, NULL, true); \
  } while(0)

/** Set attributes for parameter group with common int32 type in mag. */
#define MMS_SET_PARAMS_S32(LN, p1, p2, p3, mag_buf) \
  do { \
    MMS_SET_ATTRS_S32(LN, p1, mag_buf[0].mag, NULL, true); \
    MMS_SET_ATTRS_S32(LN, p2, mag_buf[1].mag, NULL, true); \
    MMS_SET_ATTRS_S32(LN, p3, mag_buf[2].mag, NULL, true); \
  } while(0)

// Type declarations

#if (!S2M_USE_LIBIEC)
/**
* Stub types to emulate libiec61850 internal API.
* Used when library is unavailable.
*/
struct null_s { int null; };
typedef struct null_s *IedServer;
typedef struct null_s MmsValue;
typedef struct null_s DataAttribute;
#endif //S2M_USE_LIBIEC

// Public interface function declarations

/**
* Set INT32 value of target attribute.
* Uses intermediate MmsValue variable with Integer type to update
* attribute to value
*
* @param ied IED server instance
* @param attr target attribute
* @param value new value
* @return true if attribute set, false if error occurred
*/
bool mms_if_set_attr_s32(void *ied, DataAttribute *attr, const s32_t value);

/**
* Set FLOAT value of target attribute.
* Uses intermediate MmsValue variable with Float type to update
* attribute to value
*
* @param ied IED server instance
* @param attr target attribute
* @param value new value
* @return true if attribute set, false if error occurred
*/
bool mms_if_set_attr_f32(void *ied, DataAttribute *attr, const f32_t value);

/**
* Set timestamp of target attribute.
* Uses intermediate MmsValue variable with UtcTime type to update
* attribute to value addressed by ts_ext. If argument not passed,
* uses current system time.
*
* @param ied IED server instance
* @param attr target attribute
* @param ts_ext: if using external value - pointer of type u32_t *,
*                if using system time - NULL
* @return true if attribute set, false if error occurred
*/
bool mms_if_set_attr_t(void *ied, DataAttribute *attr, const u32_t *ts_ext);

/**
* Set data quality of target attribute.
* Uses intermediate MmsValue variable with BitString type to update
* attribute to quality value
*
* @param ied IED server instance
* @param attr target attribute
* @param quality quality flag (true - good, false - bad)
* @return true if attribute set, false if error occurred
*/
bool mms_if_set_attr_q(void *ied, DataAttribute *attr, const bool quality);

#endif
