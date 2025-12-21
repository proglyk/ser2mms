/**
 * @file mms_if.h
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 09-October-2025
 */

#ifndef SER2MMS_MMS_IF_H
#define SER2MMS_MMS_IF_H

#include "ser2mms_conf.h"
#include "port_types.h"

#if (S2M_USE_LIBIEC==1)
//#include "libiec61850/api/ied_server_api.h"
#include "iec61850_server.h" // TODO "make switch by versions"
//#include "libiec61850/model/static_model.h" // TODO "shouldn't be here"
#else //S2M_USE_LIBIEC
struct null_s {
  int null;
};
typedef struct null_s *IedServer;
typedef struct null_s MmsValue;
typedef struct null_s DataAttribute;
#endif //S2M_USE_LIBIEC

// Set single attribute
#define MMS_SET_ATTR(type, PV, PX, VAL) mms_if_set_attr_##type(PV, PX, VAL)

// Base macro for setting magnitude, timestamp and quality
#define MMS_SET_ATTRS(attr_type, ggio, param, mag_suffix, c_type, mag_value, t_value, q_value) do { \
  MMS_SET_ATTR(attr_type, ied, IEDMODEL_UPG_##ggio##_##param##_##mag_suffix, (c_type)mag_value); \
  MMS_SET_ATTR(t, ied, IEDMODEL_UPG_##ggio##_##param##_t, t_value); \
  MMS_SET_ATTR(q, ied, IEDMODEL_UPG_##ggio##_##param##_q, q_value); \
} while(0)

/** @brief Wrapper for f32 type */
#define MMS_SET_ATTRS_F32(ggio, param, mag_value, t_value, q_value) \
  MMS_SET_ATTRS(f32, ggio, param, mag_f, const f32_t, mag_value, t_value, q_value)

/** @brief Wrapper for s32 type */
#define MMS_SET_ATTRS_S32(ggio, param, mag_value, t_value, q_value) \
  MMS_SET_ATTRS(s32, ggio, param, mag_i, const s32_t, mag_value, t_value, q_value)

/** @brief Super-macro for setting 3 F32 parameters in sequence */
#define MMS_SET_PARAMS_F32(ggio, p1, p2, p3, mag_buf) do { \
  MMS_SET_ATTRS_F32(ggio, p1, mag_buf[0], NULL, true); \
  MMS_SET_ATTRS_F32(ggio, p2, mag_buf[1], NULL, true); \
  MMS_SET_ATTRS_F32(ggio, p3, mag_buf[2], NULL, true); \
} while(0)

/** @brief Super-macro for setting 3 S32 parameters in sequence */
#define MMS_SET_PARAMS_S32(ggio, p1, p2, p3, mag_buf) do { \
  MMS_SET_ATTRS_S32(ggio, p1, mag_buf[0], NULL, true); \
  MMS_SET_ATTRS_S32(ggio, p2, mag_buf[1], NULL, true); \
  MMS_SET_ATTRS_S32(ggio, p3, mag_buf[2], NULL, true); \
} while(0)

/**
 * @brief Set INT32 value for specified attribute.
 * @param ied IedServer instance pointer.
 * @param attr DataAttribute to update.
 * @param value INT32 value to set.
 * @return true on success, false on failure.
 */
bool mms_if_set_attr_s32( void *, DataAttribute*, const s32_t );

/**
 * @brief Set FLOAT value for specified attribute.
 * @param ied IedServer instance pointer.
 * @param attr DataAttribute to update.
 * @param value FLOAT value to set.
 * @return true on success, false on failure.
 */
bool mms_if_set_attr_f32( void *, DataAttribute*, const f32_t );

/**
 * @brief Set TIMESTAMP value for specified attribute.
 * @param ied IedServer instance pointer.
 * @param attr DataAttribute to update.
 * @param ts_ext Pointer to external timestamp (NULL for current time).
 * @return true on success, false on failure.
 */
bool mms_if_set_attr_t( void *, DataAttribute*, const u32_t * );

/**
 * @brief Set QUALITY value for specified attribute.
 * @param ied IedServer instance pointer.
 * @param attr DataAttribute to update.
 * @param quality Boolean quality value.
 * @return true on success, false on failure.
 */
bool mms_if_set_attr_q( void *, DataAttribute*, const bool );

#endif
