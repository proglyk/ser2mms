/**
  * @file   ser2mms_conf.h
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   22-September-2025
  */
  
#ifndef SER2MMS_CONF_H
#define SER2MMS_CONF_H

#include "make_defs.h"

// #define PORT_IMPL_LINUX                 (1)
// #define PORT_IMPL_WIN32                 (2)
// #define PORT_IMPL_RTOS                  (3)
// #define PORT_IMPL_BARE                  (4)

// #define LINUX_HW_IMPL_WSL               (1)
// #define LINUX_HW_IMPL_ARM               (2)

// Enable threads
#define S2M_USE_THREADS                 (1)

// Don't use dynamically allocated objects
#define S2M_USE_STATIC                  (0)

// Var
#define S2M_USE_TRANSP_RTU              (1)
#define S2M_USE_TRANSP_TCP              (0)

// Use real libiec or imitate this one
#ifdef USE_LIBIEC
#define S2M_USE_LIBIEC                  USE_LIBIEC
#else
#define S2M_USE_LIBIEC                  (1)
#endif

// Set the type of CRC calculation method
#define CRC_YURA                        (1) // Use the reverse type
#define CRC_MODBUS                      (0) // Use the classic type

// Отладочный вывод
#define S2M_DEBUG                       (1)
// Без подписок (сокращенный режим)
#define S2M_REDUCED                     (1)

// Настройки для модуля 'ser'
#if (S2M_REDUCED==1)
#define IN_MSG_SIZE_SLAVE               (12)  //тест
#else
#define SER_NUM_SUBS                    (11)
#define IN_MSG_SIZE_SLAVE               (100) //рабочее
#endif //S2M_REDUCED
#define SER_MIN_DS_IDX                  (1)
#define SER_MAX_DS_IDX                  (6)
#define SER_MIN_PAGE_IDX                (0)
#define SER_MAX_PAGE_IDX                (3)
#define SER_PAGE_SIZE                   (3)
#define IN_MSG_SIZE_POLL                (8)
#define SER_ANSW_SIZE                   (3)

#define S2M_TRANSP_TIMEOUT              (250) // время ожидания на прием, мс

#endif //SER2MMS_CONF_H
