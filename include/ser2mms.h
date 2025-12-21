/**
  * @file   ser2mms.h
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   28-September-2025
  */

#ifndef SER2MMS_H
#define SER2MMS_H

#include "ser2mms_conf.h"
#include "transp.h" 
#include "ser.h"
#include "mms_if.h"
#include "types.h"
#include "port_rs485_init.h"

#define S2M_SLAVE                       MODE_SLAVE
#define S2M_POLL                        MODE_POLL

#define S2M_SET_PARAMS_F32              MMS_SET_PARAMS_F32
#define S2M_SET_PARAMS_S32              MMS_SET_PARAMS_S32
#define S2M_SET_ATTRS_F32               MMS_SET_ATTRS_F32
#define S2M_SET_ATTRS_S32               MMS_SET_ATTRS_S32

#define S2M_GET_TIME                    ser_get_time

typedef struct ser2mms_s ser2mms_t;
typedef ser2mms_t        s2m_t;
//typedef topmode_t        s2m_mode_t;

/**
  * @brief  Constructor
  * @retval self - Pointer to the object itself
  */
s2m_t *ser2mms_new(void *, u32_t, u32_t, void *);

/**
  * @brief  Launcher
  * @param  self - Pointer to the object itself
  */
s32_t ser2mms_run(s2m_t *);

/**
  * @brief  Infinite polling
  * @param  self - Pointer to the object itself
  */
void ser2mms_poll(s2m_t *);

/**
  * @brief  Destructor
  * @param  self - Pointer to the object itself
  */
void ser2mms_stop(s2m_t *);

/**
  * @brief  For test
  * @param  self - Pointer to the object itself
  */
void ser2mms_test_tick(s2m_t *);

/**
  * @brief  Top layer's 'cmd' field setter
  * @param  self - Pointer to the object itself
  * @param  cmd - New value of 'cmd'
  */
void ser2mms_set_cmd(s2m_t *, u32_t);

/**
  * @brief  Top layer's 'id' field setter
  * @param  self - Pointer to the object itself
  * @param  id - New value of 'id'
  */
void ser2mms_set_id(s2m_t *, u32_t);

/**
  * @brief  Top layer's 'id' field setter
  * @param  self - Pointer to the object itself
  * @retval Opaaue pointer to IED server instance
  */
void *ser2mms_get_ied(s2m_t *);

/**
 * @brief ???
 */
void ser2mms_read_carg(void *opaque, u16_t *carg_buf, __UNUSED u32_t carg_len, u8_t ds, u8_t page);

/**
 * @brief ???
 */
void ser2mms_read_subs(void *opaque, prm_t *subs_buf, __UNUSED u32_t subs_len);

/**
 * @brief ???
 */
void ser2mms_write_answer(void *argv, u16_t *answ_buf, u32_t *answ_len);

/**
 * @brief Получение системного времени (тело функции реализуется пользователем)
 * @param epoch Место под размещение времени в формате epoch-time
 * @param usec Место под размещение микросекундной части
 */
void ser_set_time(uint32_t *epoch, uint32_t *usec);


/* void *mb__get_ser_h(mb_t *);
void *mb__get_tim_h(mb_t *);
void  mb__recv_cb(mb_t *);
void  mb__tick_cb(mb_t *);
void  mb__test_recv(mb_t *); */

#endif
