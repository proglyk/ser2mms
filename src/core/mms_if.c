/**
 * @file mms_if.c
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 09-October-2025
 */

#include "mms_if.h"
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#if (S2M_USE_LIBIEC==0)
static void MmsValue_delete(MmsValue* mms); // TODO
static bool IedServer_updateAttributeValue(void *, DataAttribute *, MmsValue *); // TODO
static MmsValue* MmsValue_newIntegerFromInt32(s32_t); // TODO
static MmsValue* MmsValue_newFloat(f32_t); // TODO
static MmsValue* MmsValue_newBitString(u32_t argc);
static void MmsValue_setBitStringBit(MmsValue *mms, int, bool);
static MmsValue *MmsValue_newUtcTimeByTimestamp(u32_t *);
#endif//S2M_USE_LIBIEC

// ============================= Public functions ==============================

/**
 * @brief Set INT32 value for specified attribute.
 */
bool mms_if_set_attr_s32( void *argv, DataAttribute* attr, const s32_t value )
{
  IedServer ied = (IedServer)argv;
  assert(ied && attr);
  // Local MmsValue entity
  MmsValue* pValueMms = MmsValue_newIntegerFromInt32(value);
  // If memory allocation failed, exit
  if (!pValueMms) return false;
  // Update attribute
  //bool result = IedServer_updateAttributeValue(ied, attr, pValueMms);
  IedServer_updateAttributeValue(ied, attr, pValueMms); // TODO "removed bool"
  // Free resource
  MmsValue_delete(pValueMms);
  return true;
}

/**
 * @brief Set FLOAT value for specified attribute.
 */
bool mms_if_set_attr_f32( void *argv, DataAttribute* attr, const f32_t value )
{
  IedServer ied = (IedServer)argv;
  assert(ied && attr);
  // Local MmsValue entity
  MmsValue* pValueMms = MmsValue_newFloat(value);
  // If memory allocation failed, exit
  if (!pValueMms) return false;
  // Update attribute
  // bool result = IedServer_updateAttributeValue(ied, attr, pValueMms);
  IedServer_updateAttributeValue(ied, attr, pValueMms); // TODO "removed bool"
  // Free resource
  MmsValue_delete(pValueMms);
  return true;
}

/**
 * @brief Set timestamp value for specified attribute.
 */
bool mms_if_set_attr_t( void *argv, DataAttribute *attr, const u32_t *ts_ext )
{
  IedServer ied = (IedServer)argv;
  uint32_t ts_int[2];
  // All three arguments are not validated further
  assert(ied && attr);
  if (!ts_ext) {
#if (PORT_IMPL==PORT_IMPL_LINUX)
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);
    ts_int[0] = tspec.tv_sec;
    ts_int[1] = tspec.tv_nsec / 1000;
#elif (PORT_IMPL==PORT_IMPL_RTOS)
#error "Not available"
    // Get time from clock
    //GET_SYSTEM_TIME((timestamp+0),(timestamp+1));
#elif (PORT_IMPL==PORT_IMPL_BARE)
#error "Not available"
    // Get time from clock
    //GET_SYSTEM_TIME((timestamp+0),(timestamp+1));
#else
#error "PORT_IMPL must be defined"
#endif //PORT_IMPL==...
  }
  // Save it in MmsValue variable
  // if ( (MmsValue* pTimeMms = MmsValue_newUtcTime(ts_int[0])) == 0) {
  //   return false;
  // }
  MmsValue* pTimeMms = MmsValue_newUtcTime(ts_int[0]);
  if (!pTimeMms) {
    return false;
  }
  // MmsValue* pTimeMms = MmsValue_newUtcTimeByTimestamp(
  //   ts_ext ? (u32_t *)ts_ext : ts_int);
  // If memory allocation failed, exit
  // if (!pTimeMms) return false;
  // Update attribute
  IedServer_updateAttributeValue(ied, attr, pTimeMms);
  // Free resource
  MmsValue_delete(pTimeMms);
  return true;
}

/**
 * @brief Set quality value for specified attribute.
 */
bool mms_if_set_attr_q( void *argv, DataAttribute* attr, const bool quality )
{
  IedServer ied = (IedServer)argv;
  assert(ied && attr);
  // Local MmsValue entity
  MmsValue* pQualityMms = MmsValue_newBitString(13);
  // If memory allocation failed, exit
  if (!pQualityMms)
    return false;
  // Write true value to bit position 0
  MmsValue_setBitStringBit(pQualityMms, 0, quality);
  // Update attribute
  IedServer_updateAttributeValue(ied, attr, pQualityMms);
  // Free resource
  MmsValue_delete(pQualityMms);
  return true;
}

// ============================= Static functions ==============================

#if (S2M_USE_LIBIEC==0)
static MmsValue *MmsValue_newFloat(f32_t value)
{
  (void)value;
  return NULL;
}

static MmsValue *MmsValue_newIntegerFromInt32(s32_t value)
{
  (void)value;
  return NULL;
}

static bool IedServer_updateAttributeValue(void *ied, DataAttribute *attr,
                                            MmsValue *mms)
{
  (void)ied;
  (void)attr;
  (void)mms;
  return false;
}

static MmsValue *MmsValue_newBitString(u32_t argc)
{
  (void)argc;
  return NULL;
}

static void MmsValue_setBitStringBit(MmsValue *mms, int value, bool q)
{
  (void)mms;
  (void)value;
  (void)q;
}

static MmsValue *MmsValue_newUtcTimeByTimestamp(u32_t *timestamp)
{
  (void)timestamp;
  return NULL;
}

static void MmsValue_delete(MmsValue *mms)
{
  (void)mms;
}

#endif//LIBIEC_EXIST
