/**
  * @file   port_conf.h
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   22-September-2025
  */
  
#ifndef PORT_CONF_H
#define PORT_CONF_H

#include "make_defs.h"

// #define PORT_IMPL_LINUX                 (1)
// #define PORT_IMPL_WIN32                 (2)
// #define PORT_IMPL_RTOS                  (3)
// #define PORT_IMPL_BARE                  (4)

// #define LINUX_HW_IMPL_WSL               (5)
// #define LINUX_HW_IMPL_ARM               (6)

// #ifdef ARM_RTOS
// #elif defined(ARM_LINUX)
// #elif defined(x86_64_LINUX)
//#define x86_64_LINUX                    (1)
// #endif

// Enable threads
#define PORT_USE_THREADS                (1)

// Use dynamically allocated objects
#define PORT_USE_STATIC                 (1)

// Enable debug mode
#define PORT_DBG_EN                     (1)

// Time to lock nRE/DE in push-up, ms
#define PORT_RS485_DE_WAIT              (2)

#endif //PORT_CONF_H
