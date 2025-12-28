/**
* @file ser2mms_conf.h
* @author Ilia Proniashin, msg@proglyk.ru
* @date 22-September-2025
*
* Configuration header for ser2mms module.
*/

#ifndef SER2MMS_CONF_H
#define SER2MMS_CONF_H

#include "make_defs.h"

// #define PORT_IMPL_LINUX (1)
// #define PORT_IMPL_WIN32 (2)
// #define PORT_IMPL_RTOS (3)
// #define PORT_IMPL_BARE (4)

// #define LINUX_HW_IMPL_WSL (1)
// #define LINUX_HW_IMPL_ARM (2)

/** Use threads. */
#define S2M_USE_THREADS                 (1)

/** Use static allocation. */
#define S2M_USE_STATIC                  (0)

/** Implementation selection for 'transp' interface. */
#define S2M_USE_TRANSP_RTU              (1) // Use RTU
#define S2M_USE_TRANSP_TCP              (0) // Use TCP

/** Use full LIBIEC library API or emulate it. */
#ifdef LIBIEC
#define S2M_USE_LIBIEC                  LIBIEC
#else
#define S2M_USE_LIBIEC                  (1)
#endif

/** Use API of old LIBIEC versions. */
#define S2M_USE_OLD_LIBIEC_API          (0)

/** CRC calculation method selection */
#define CRC_MODBUS                      (0) // Classic variant
#define CRC_YURA                        (1) // Reverse variant

// Debug output
#define S2M_DEBUG                       (1)

// Without subscriptions (reduced mode)
#define S2M_REDUCED                     (0)

#endif //SER2MMS_CONF_H
