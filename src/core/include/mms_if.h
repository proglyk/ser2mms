/**
  * @file   mms_if.h
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   09-October-2025
  */

#ifndef SER2MMS_MMS_IF_H
#define SER2MMS_MMS_IF_H

#include "ser2mms_conf.h"
#include "port_types.h"

#if (S2M_USE_LIBIEC==1)
#include "libiec61850/api/ied_server_api.h"
#include "libiec61850/model/static_model.h"
#else //S2M_USE_LIBIEC
struct null_s {
  int null;
};
typedef struct null_s *IedServer;
typedef struct null_s MmsValue;
typedef struct null_s DataAttribute;
#endif //S2M_USE_LIBIEC

// ������-���������
#define MMS_SET_ATTR(type, PV, PX, VAL) mms_if_set_attr_##type(PV, PX, VAL)

/**
  * @brief ������� �������� ���� INT32 ��� ���������� ���������.
  * @param dataset: ����� �������� �������� �������.
  * @param cargpage: ����� ������� �������� �������.
  * @param ptr: ��������� �� ��������� �� ���������� �� �������.
  */
bool mms_if_set_attr_s32( void *, DataAttribute*, const s32_t );

/**
  * @brief ������� �������� ���� FLOAT ��� ���������� ���������.
  * @param dataset: ����� �������� �������� �������.
  * @param cargpage: ����� ������� �������� �������.
  * @param ptr: ��������� �� ��������� �� ���������� �� �������.
  */
bool mms_if_set_attr_f32( void *, DataAttribute*, const f32_t );

/**
  * @brief ������� �������� ���� ������� TIMESTAMP ��� ���������� ���������.
  * @param dataset: ����� �������� �������� �������.
  * @param cargpage: ����� ������� �������� �������.
  * @param ptr: ��������� �� ��������� �� ���������� �� �������.
  */
bool mms_if_set_attr_t( void *, DataAttribute*, const u32_t * );

/**
  * @brief ������� �������� ���� �������� QUALITY ��� ���������� ���������.
  * @param dataset: ����� �������� �������� �������.
  * @param cargpage: ����� ������� �������� �������.
  * @param ptr: ��������� �� ��������� �� ���������� �� �������.
  */
bool mms_if_set_attr_q( void *, DataAttribute*, const bool );

#endif
