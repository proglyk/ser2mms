/**
 * @file byteops.h
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 05-November-2025
 * 
 * Byte operation macros.
 * Set of macros for converting between bytes and multi-byte data types
 * considering byte order (big-endian/little-endian).
 */

#ifndef SER2MMS_MACRO_H
#define SER2MMS_MACRO_H

#include "port_types.h"

/**
 * Convert two bytes to short integer (Byte TO Short).
 * Combines two bytes passed in arguments a,b into one 16-bit value
 * with big-endian order (2 x s8_t => s16_t).
 * 
 * @param a high byte
 * @param b low byte
 * @return 16-bit value
 */
#define B_TO_S(a,b) ((((a) << 8) & 0xff00) | \
                     ((b) & 0x00ff))

/**
 * Convert two bytes to short integer (Byte* TO Short).
 * Combines two bytes taken from array addressed by pointer p into one
 * 16-bit value with big-endian order (2 x s8_t* => s16_t).
 * 
 * @param p pointer to byte array
 * @return 16-bit value
 */
#define PB_TO_S(p) ((((p)[0] << 8) & 0xff00) | \
                    ((p)[1] & 0x00ff))

/**
 * Write short integer to byte array (Short TO Byte*).
 * Converts 16-bit value to 2 bytes of array addressed by
 * pointer p, with big-endian order (s16_t => 2 x s8_t*).
 * 
 * @param p pointer to byte array (minimum 2 bytes)
 * @param b 16-bit value to write
 */
#define S_TO_PB(p,b) \
  do { \
    (p)[0] = (u8_t)(((b) & 0xff00)>>8); \
    (p)[1] = (u8_t)( (b) & 0x00ff); \
  } while(0)

/**
 * Write short integer to byte array (Short TO swapped Byte*).
 * Converts 16-bit value to 2 bytes of array addressed by
 * pointer p, with reversed order (little-endian).
 * 
 * @param p pointer to byte array (minimum 2 bytes)
 * @param b 16-bit value to write
 */
#define S_TO_swPB(p,b) \
  do { \
    (p)[1] = (u8_t)(((b) & 0xff00)>>8); \
    (p)[0] = (u8_t)( (b) & 0x00ff); \
  } while(0)

/**
 * Write integer to byte array (Int TO Byte*).
 * Converts 32-bit value to 4 bytes of array addressed by
 * pointer p, with big-endian order (s32_t => 4 x s8_t*).
 * 
 * @param p pointer to byte array (minimum 4 bytes)
 * @param i 32-bit value to write
 */
#define I_TO_PB(p,i) \
  do { \
    (p)[0] = (u8_t)(((i) & 0xFF000000) >> 24); \
    (p)[1] = (u8_t)(((i) & 0x00FF0000) >> 16); \
    (p)[2] = (u8_t)(((i) & 0x0000FF00) >> 8); \
    (p)[3] = (u8_t)( (i) & 0x000000FF); \
  } while(0)

/**
 * Write integer to byte array (Int TO swapped Byte*).
 * Converts 32-bit value to 4 bytes of array addressed by
 * pointer p, with reversed order (little-endian).
 * 
 * @param p pointer to byte array (minimum 4 bytes)
 * @param i 32-bit value to write
 */
#define I_TO_swPB(p,i) \
  do { \
    (p)[3] = (u8_t)(((i) & 0xFF000000) >> 24); \
    (p)[2] = (u8_t)(((i) & 0x00FF0000) >> 16); \
    (p)[1] = (u8_t)(((i) & 0x0000FF00) >> 8); \
    (p)[0] = (u8_t)( (i) & 0x000000FF); \
  } while(0)

/**
 * Convert four bytes to long integer (big-endian).
 * Combines 4 bytes into 32-bit value with order from most to least significant byte.
 * Order: big-endian.
 * 
 * @param a most significant byte (bits 24-31)
 * @param b second byte (bits 16-23)
 * @param c third byte (bits 8-15)
 * @param d least significant byte (bits 0-7)
 * @return 32-bit value
 */
#define B_TO_L(a,b,c,d) ((((a) << 24) & 0xff000000UL) | \
                         (((b) << 16) & 0x00ff0000UL) | \
                         (((c) << 8) & 0x0000ff00UL) | \
                         ( (d) & 0x000000ffUL))

#endif
