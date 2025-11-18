/**
 * @file port_alloc.h
 * @brief Unified allocation macros for port modules
 * @author Ilia Proniashin
 */
#ifndef PORT_ALLOC_H
#define PORT_ALLOC_H

#include <string.h>
#include <stdlib.h>

// ==============================================================================
// ¬нутренние макросы дл¤ конкатенации имЄн
// ==============================================================================
#define _PORT_CONCAT(a, b) a##b
#define PORT_CONCAT(a, b) _PORT_CONCAT(a, b)

// ==============================================================================
// ѕубличные макросы
// ==============================================================================

/**
 * @brief Declare static storage for module
 * —оздаЄт статические переменные _<module>_self и _<module>_self_is_used
 * @param module Module name (THREAD, RS485, TIMER, etc.)
 * @param type Structure type (struct thread_s, struct rs485_s, etc.)
 */
// #define PORT_STATIC_DECLARE((mod),(type)) \
                                        // do { \
                                          // static (type) _##(mod)##_self; \
                                          // static int _##(mod)##_self_is_used = 0; \
                                        // while(0);
#define PORT_STATIC_DECLARE(mod, type)  static type _##mod##_self; \
                                        static int _##mod##_self_is_used = 0;

/**
 * @brief Allocate object (static or dynamic)
 * јвтоматически использует флаг <module>_USE_STATIC дл¤ выбора типа аллокации
 * @param module Module name (THREAD, RS485, TIMER, etc.)
 * @param type Structure type
 * @param ptr_name Variable name for pointer
 * @param on_error Action on allocation failure (e.g., return NULL)
 */
#define PORT_ALLOC(module, type, ptr_name, on_error) \
    type *ptr_name; \
    do { \
        if (PORT_CONCAT(module, _USE_STATIC)) { \
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
 * @brief Free object
 * јвтоматически использует флаг <module>_USE_STATIC
 * @param module Module name (THREAD, RS485, TIMER, etc.)
 * @param ptr Pointer to object
 */
#define PORT_FREE(module, ptr) \
    do { \
        if (PORT_CONCAT(module, _USE_STATIC)) { \
            _##module##_self_is_used = 0; \
        } else { \
            free(ptr); \
        } \
    } while(0)

#endif // PORT_ALLOC_H
