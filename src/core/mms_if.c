/**
 * @file mms_if.c
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 09-October-2025
 * 
 * MMS attribute interface implementation.
 */

#include "mms_if.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>

// Private function declarations

#if (!S2M_USE_LIBIEC)
// Private function prototypes for libiec61850 API emulation
static MmsValue* MmsValue_newIntegerFromInt32(s32_t);
static MmsValue* MmsValue_newFloat(f32_t);
static MmsValue* MmsValue_newBitString(u32_t argc);
static void MmsValue_setBitStringBit(MmsValue *mms, int, bool);
static void MmsValue_delete(MmsValue* mms);
#if (S2M_USE_OLD_LIBIEC_API)
static bool IedServer_updateAttributeValue(void *, DataAttribute *, MmsValue *);
static MmsValue *MmsValue_newUtcTimeByTimestamp(u32_t *);
#else
static void IedServer_updateAttributeValue(void *, DataAttribute *, MmsValue *);
static MmsValue *MmsValue_newUtcTime(uint32_t timeval);
#endif
#endif

// Public interface function definitions

/**
 * Set INT32 value of target attribute.
 */
bool mms_if_set_attr_s32(void *argv, DataAttribute* attr, const s32_t value)
{
  bool rc = true;
  IedServer ied = (IedServer)argv;
  assert(ied && attr);

  // Create local MmsValue entity
  MmsValue* pValueMms = MmsValue_newIntegerFromInt32(value);
  if (!pValueMms) return false;

  // Update attribute
#if (S2M_USE_OLD_LIBIEC_API)
  rc = IedServer_updateAttributeValue(ied, attr, pValueMms);
#else
  IedServer_updateAttributeValue(ied, attr, pValueMms);
#endif

  // Free resource
  MmsValue_delete(pValueMms);
  return rc;
}

/**
 * Set FLOAT value of target attribute.
 */
bool mms_if_set_attr_f32(void *argv, DataAttribute* attr, const f32_t value)
{
  bool rc = true;
  IedServer ied = (IedServer)argv;
  assert(ied && attr);

  // Create local MmsValue entity
  MmsValue* pValueMms = MmsValue_newFloat(value);
  if (!pValueMms) return false;

  // Update attribute
#if (S2M_USE_OLD_LIBIEC_API)
  rc = IedServer_updateAttributeValue(ied, attr, pValueMms);
#else
  IedServer_updateAttributeValue(ied, attr, pValueMms);
#endif

  // Free resource
  MmsValue_delete(pValueMms);
  return rc;
}

/**
 * Set timestamp of target attribute.
 */
bool mms_if_set_attr_t(void *argv, DataAttribute *attr, const u32_t *ts_ext)
{
  bool rc = true;
  IedServer ied = (IedServer)argv;
  uint32_t ts_int[2];
  assert(ied && attr);

  if (!ts_ext) {
#if (PORT_IMPL==PORT_IMPL_LINUX)
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);
    ts_int[0] = tspec.tv_sec;
    ts_int[1] = tspec.tv_nsec / 1000;
#elif (PORT_IMPL==PORT_IMPL_RTOS)
    #error "Not available"
    // Get time from RTC
#elif (PORT_IMPL==PORT_IMPL_BARE)
    #error "Not available"
    // Get time from RTC
#else
    #error "PORT_IMPL must be defined"
#endif
  }

  // Create timestamp MmsValue
  MmsValue* pTimeMms = MmsValue_newUtcTime(ts_int[0]);
  if (!pTimeMms) return false;

  // Update attribute
#if (S2M_USE_OLD_LIBIEC_API)
  rc = IedServer_updateAttributeValue(ied, attr, pTimeMms);
#else
  IedServer_updateAttributeValue(ied, attr, pTimeMms);
#endif

  // Free resource
  MmsValue_delete(pTimeMms);
  return rc;
}

/**
 * Set data quality of target attribute.
 */
bool mms_if_set_attr_q(void *argv, DataAttribute* attr, const bool quality)
{
  bool rc = true;
  IedServer ied = (IedServer)argv;
  assert(ied && attr);

  // Create local MmsValue entity (13-bit bit string)
  MmsValue* pQualityMms = MmsValue_newBitString(13);
  if (!pQualityMms) return false;

  // Set value at bit position 0
  MmsValue_setBitStringBit(pQualityMms, 0, quality);

  // Update attribute
#if (S2M_USE_OLD_LIBIEC_API)
  rc = IedServer_updateAttributeValue(ied, attr, pQualityMms);
#else
  IedServer_updateAttributeValue(ied, attr, pQualityMms);
#endif

  // Free resource
  MmsValue_delete(pQualityMms);
  return rc;
}

#if (!S2M_USE_LIBIEC)
/**
 * Stub functions for libiec61850 internal API emulation.
 * Used when library is unavailable.
 */
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

static void MmsValue_delete(MmsValue *mms)
{
  (void)mms;
}

#if (S2M_USE_OLD_LIBIEC_API)
static MmsValue *MmsValue_newUtcTimeByTimestamp(u32_t *timestamp)
{
  (void)timestamp;
  return NULL;
}

static bool
IedServer_updateAttributeValue(void *ied, DataAttribute *attr, MmsValue *mms)
{
  (void)ied;
  (void)attr;
  (void)mms;
  return false;
}
#else // S2M_USE_OLD_LIBIEC_API
static MmsValue *MmsValue_newUtcTime(uint32_t timeval)
{
  (void)timeval;
  return NULL;
}

static void
IedServer_updateAttributeValue(void *ied, DataAttribute *attr, MmsValue *mms)
{
  (void)ied;
  (void)attr;
  (void)mms;
}
#endif // S2M_USE_OLD_LIBIEC_API
#endif // S2M_USE_LIBIEC
