/**
  * @file   macro.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   05-November-2025
  */

#ifndef  SER2MMS_MACRO_H
#define  SER2MMS_MACRO_H

#include "port_types.h"

// Byte TO Short (s8_t => s16_t)
#define B_TO_S(a,b)                     ((((a) << 8) & 0xff00) | \
                                          ((b) & 0x00ff))

// Byte* TO Short (s8_t* => s16_t)
#define PB_TO_S(p)                      ((((p)[0] << 8) & 0xff00) | \
                                          ((p)[1] & 0x00ff))

// Short TO Byte* (1 x s16_t => 2 x s8_t*)
#define S_TO_PB(p,b)                    do { \
                                          (p)[0] = (u8_t)(((b) & 0xff00)>>8); \
                                          (p)[1] = (u8_t)((b) & 0x00ff); \
                                        } while(0)

// Short TO swapped Byte* (s16_t => s8_t*)
#define S_TO_swPB(p,b)                  do { \
                                          (p)[1] = (u8_t)(((b) & 0xff00)>>8); \
                                          (p)[0] = (u8_t)((b) & 0x00ff); \
                                        } while(0)

// Int TO Byte* (1 x s32_t => 4 x s8_t*)
#define I_TO_PB(p,i)              do { \
                                    (p)[0] = (u8_t)(((i) & 0xFF000000) >> 24); \
                                    (p)[1] = (u8_t)(((i) & 0x00FF0000) >> 16); \
                                    (p)[2] = (u8_t)(((i) & 0x0000FF00) >> 8);  \
                                    (p)[3] = (u8_t)((i) & 0x000000FF); \
                                  } while(0)
                    
#define SUB_TO_DS(a)                    (((a) >> 4) & 0x0f)
#define DS_TO_B(a)                      (((a) << 4) & 0xf0)

#define BTOSUB(a,b)                     ((((a) << 8) & 0x0000ff00) | \
                                         (((b) << 0) & 0x000000ff))

#define B_TO_PG(a)                      ((a) & 0x0f)
                     
#define B_TO_L(a,b,c,d)                 ((((a) << 24) & 0xff000000UL) | \
                                         (((b) << 16) & 0x00ff0000UL) | \
                                         (((c) << 8) & 0x0000ff00UL)  | \
                                          ((d) & 0x000000ffUL))

#endif