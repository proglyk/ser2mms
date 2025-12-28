/**
* @file ser2mms_defs.h
* @author Ilia Proniashin, msg@proglyk.ru
* @date 28-December-2025
*
* Sample defines.
*/

#ifndef SER2MMS_DEFS_H
#define SER2MMS_DEFS_H

#include "ser2mms.h"

#if (!S2M_USE_LIBIEC)
static DataAttribute iedModel_UPG_GGIO0_ConnStatus_mag_i;
static DataAttribute iedModel_UPG_GGIO0_ConnStatus_q;
static DataAttribute iedModel_UPG_GGIO0_ConnStatus_t;
#define IEDMODEL_UPG_GGIO0_ConnStatus_mag_i (&iedModel_UPG_GGIO0_ConnStatus_mag_i)
#define IEDMODEL_UPG_GGIO0_ConnStatus_q (&iedModel_UPG_GGIO0_ConnStatus_q)
#define IEDMODEL_UPG_GGIO0_ConnStatus_t (&iedModel_UPG_GGIO0_ConnStatus_t)

static DataAttribute iedModel_UPG_GGIO0_HV_mag_i;
static DataAttribute iedModel_UPG_GGIO0_HV_q;
static DataAttribute iedModel_UPG_GGIO0_HV_t;
#define IEDMODEL_UPG_GGIO0_HV_mag_i (&iedModel_UPG_GGIO0_HV_mag_i)
#define IEDMODEL_UPG_GGIO0_HV_q (&iedModel_UPG_GGIO0_HV_q)
#define IEDMODEL_UPG_GGIO0_HV_t (&iedModel_UPG_GGIO0_HV_t)

static DataAttribute iedModel_UPG_GGIO0_LV_mag_i;
static DataAttribute iedModel_UPG_GGIO0_LV_q;
static DataAttribute iedModel_UPG_GGIO0_LV_t;
#define IEDMODEL_UPG_GGIO0_LV_mag_i (&iedModel_UPG_GGIO0_LV_mag_i)
#define IEDMODEL_UPG_GGIO0_LV_q (&iedModel_UPG_GGIO0_LV_q)
#define IEDMODEL_UPG_GGIO0_LV_t (&iedModel_UPG_GGIO0_LV_t)
#endif // S2M_USE_LIBIEC

#endif //SER2MMS_DEFS_H
