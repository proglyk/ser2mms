/**
  * @file   port_types.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   12-October-2025
  */

#ifndef PORT_TYPES_H
#define PORT_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// char 1 byte
typedef int8_t    s8_t;
typedef uint8_t   u8_t;

// short 2 bytes
typedef int16_t   s16_t;
typedef uint16_t  u16_t;

// long 4 bytes
typedef int32_t   s32_t;
typedef uint32_t  u32_t;

// long 8 bytes
typedef int64_t   s64_t;
typedef uint64_t  u64_t;

typedef float     f32_t;

// 
typedef void (*fn_t)(void *);

// File Descriptor
typedef s32_t     fd_t;

#if defined (__GNUC__)
#define __UNUSED __attribute__((__unused__))
#define __PACKED __attribute__((__packed__))
#define __WEAK   __attribute__((weak))
#elif defined (__ICCARM__)
#define __UNUSED
#define __PACKED
#define __WEAK   __weak
#endif

#endif /*PORT_TYPES_H*/