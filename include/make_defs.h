/**
  * @file   make_defs.h
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   06-December-2025
  */
  
#ifndef MAKE_DEFS_H
#define MAKE_DEFS_H

// Возможные значения PORT_IMPL
#define LINUX                           (1)
#define WIN32                           (2)
#define RTOS                            (3)
#define BARE                            (4)
// C префиксами для ясности:
#define PORT_IMPL_LINUX                 LINUX
#define PORT_IMPL_WIN32                 WIN32
#define PORT_IMPL_RTOS                  RTOS
#define PORT_IMPL_BARE                  BARE

// Возможные значения LINUX_HW_IMPL
#define WSL                             (10)
#define ARM                             (20)
// C префиксами для ясности:
#define LINUX_HW_IMPL_WSL               WSL
#define LINUX_HW_IMPL_ARM               ARM

// // Возможные значения LIBIEC
// #define LINUX                           (1)
// // C префиксами для ясности:
// #define PORT_IMPL_LINUX                 LINUX

#endif //MAKE_DEFS_H
