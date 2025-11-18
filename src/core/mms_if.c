/**
  * @file   mms_if.c
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   09-October-2025
  */

#include "mms_if.h"
#include <stdlib.h>

#if (!LIBIEC_EXIST)
  
static void MmsValue_delete(MmsValue* mms); // TODO
static bool IedServer_updateAttributeValue(void *, DataAttribute *, MmsValue *); // TODO
static MmsValue* MmsValue_newIntegerFromInt32(s32_t); // TODO
static MmsValue* MmsValue_newFloat(f32_t); // TODO
static MmsValue* MmsValue_newBitString(u32_t argc);
static void MmsValue_setBitStringBit(MmsValue *mms, int, bool);
static MmsValue *MmsValue_newUtcTimeByTimestamp(u32_t *);

#endif//LIBIEC_EXIST

/**
  * @brief Функция обновления полей датасетов.
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  */
bool mms_set_attr_s32( void *argv, DataAttribute* attr, s32_t value )
{
  IedServer ied = (IedServer)argv;
  bool result = false;
  
  // локальная сущность MmsValue
  MmsValue* pValueMms = MmsValue_newIntegerFromInt32(value);
  // если не удалось выделелить память, то выходим
  if (!pValueMms) return false;
  // Обновляем аттрибут
  result = IedServer_updateAttributeValue(ied, attr, pValueMms);
  // освобождаем ресурс
  MmsValue_delete(pValueMms);
  
  return result;
}

/**
  * @brief Функция обновления полей датасетов.
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  */
bool mms_set_attr_f32( void *argv, DataAttribute* attr, f32_t value )
{
  IedServer ied = (IedServer)argv;
  bool result = false;
  
  // локальная сущность MmsValue
  MmsValue* pValueMms = MmsValue_newFloat(value);
  // если не удалось выделелить память, то выходим
  if (!pValueMms) return false;
  // Обновляем аттрибут
  result = IedServer_updateAttributeValue(ied, attr, pValueMms);
  // освобождаем ресурс
  MmsValue_delete(pValueMms);
  
  return result;
}

/**
  * @brief Функция задания поля времени указанного аттрибута.
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  */
bool mms_set_attr_t( void *argv, DataAttribute *attr, u32_t *pTimestamp )
{
  IedServer ied = (IedServer)argv;
  //uint32_t timestamp[2] = {0,0};
  MmsValue* pTimeMms = NULL;
  
  // локальная сущность MmsValue
  //MmsValue* pValueMms = MmsValue_newIntegerFromInt32(value);
  
  // Получить время с часов
  //GET_SYSTEM_TIME((timestamp+0),(timestamp+1));
  // сохранить его в переменной типа MmsValue
  pTimeMms = MmsValue_newUtcTimeByTimestamp(pTimestamp);
  
  // если не удалось выделелить память, то выходим
  if (!pTimeMms) return false;
  
  // Обновляем аттрибут
  IedServer_updateAttributeValue(ied, attr, pTimeMms);
  // освобождаем ресурс
  MmsValue_delete(pTimeMms);
  
  return true;
}

/**
  * @brief Функция задания поля времени указанного аттрибута.
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  */
bool mms_set_attr_q( void *argv, DataAttribute* attr, bool quality )
{
  IedServer ied = (IedServer)argv;
  
  // локальная сущность MmsValue
  MmsValue* pQualityMms = MmsValue_newBitString(13);
  // если не удалось выделелить память, то выходим
  if (!pQualityMms)
    return false;
  // пишем значение true в битовую позицию 0.
  MmsValue_setBitStringBit(pQualityMms, 0, quality);
  // Обновляем аттрибут
  IedServer_updateAttributeValue(ied, attr, pQualityMms);
  // освобождаем ресурс
  MmsValue_delete(pQualityMms);
  
  return true;
}

#if (!LIBIEC_EXIST)

static MmsValue *MmsValue_newFloat(__UNUSED f32_t value)
{
  return NULL;
}

static MmsValue *MmsValue_newIntegerFromInt32(__UNUSED s32_t value)
{
  return NULL;
}

static bool IedServer_updateAttributeValue(__UNUSED void *ied, 
                                           __UNUSED DataAttribute *attr,
                                           __UNUSED MmsValue *mms)
{
  return false;
}

static MmsValue *MmsValue_newBitString(__UNUSED u32_t argc)
{
  return NULL;
}

static void MmsValue_setBitStringBit(__UNUSED MmsValue *mms, 
                                     __UNUSED int value, __UNUSED bool q)
{
  
}

static MmsValue *MmsValue_newUtcTimeByTimestamp(__UNUSED u32_t *timestamp)
{
  return NULL;
}

static void MmsValue_delete(__UNUSED MmsValue *mms)
{
  
}

#endif//LIBIEC_EXIST