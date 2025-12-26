/**
 * @file alloc.h
 * @author Ilia Proniashin
 * @date 30-October-2025
 * 
 * Memory allocation macros.
 * Provides unified macros for static and dynamic memory allocation,
 * allowing compile-time selection of allocation strategy per module.
 */

#ifndef ALLOC_H
#define ALLOC_H

#include <string.h>
#include <stdlib.h>

// Internal macros for name concatenation
#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

/**
 * Declare static storage for module.
 * Creates static variables _<module>_self and _<module>_self_is_used.
 * 
 * @param module Module name (THREAD, RS485, TIMER, etc.)
 * @param type Structure type (struct thread_s, struct rs485_s, etc.)
 */
#define STATIC_DECLARE(mod, type) static type _##mod##_self; \
  static int _##mod##_self_is_used = 0;

/**
 * Allocate memory for object (statically or dynamically).
 * Automatically uses <module>_USE_STATIC flag to select allocation type.
 * 
 * @param module Module name (THREAD, RS485, TIMER, etc.)
 * @param type Structure type
 * @param ptr_name Variable name for pointer
 * @param on_error Action on allocation error (e.g., return NULL)
 */
#define ALLOC(module, type, ptr_name, on_error) \
  type *ptr_name; \
  do { \
    if (CONCAT(module, _USE_STATIC)) { \
      if (_##module##_self_is_used) { on_error; } \
      _##module##_self_is_used = 1; \
      ptr_name = &_##module##_self; \
      memset((void*)ptr_name, 0, sizeof(type)); \
    } else { \
      ptr_name = calloc(1, sizeof(type)); \
      if (!ptr_name) { on_error; } \
    } \
  } while(0)

/**
 * Free object.
 * Automatically uses <module>_USE_STATIC flag.
 * 
 * @param module Module name (THREAD, RS485, TIMER, etc.)
 * @param ptr Pointer to object
 */
#define FREE(module, ptr) \
  do { \
    if (CONCAT(module, _USE_STATIC)) { \
      _##module##_self_is_used = 0; \
    } else { \
      free(ptr); \
    } \
  } while(0)

#endif // ALLOC_H
