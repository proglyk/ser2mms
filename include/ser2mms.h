/**
* @file ser2mms.h
* @author Ilia Proniashin, msg@proglyk.ru
* @date 28-September-2025
*/

#ifndef SER2MMS_H
#define SER2MMS_H

#include "ser2mms_conf.h"
#include "transp.h"
#include "ser.h"
#include "mms_if.h"
#include "port_rs485_init.h"

/** Use static allocation. */
#define SER2MMS_USE_STATIC (0) //S2M_USE_STATIC

/** Renaming internal macros to external style. */
#define S2M_SLAVE MODE_SLAVE
#define S2M_POLL MODE_POLL
#define S2M_SET_PARAMS_F32 MMS_SET_PARAMS_F32
#define S2M_SET_PARAMS_S32 MMS_SET_PARAMS_S32
#define S2M_SET_ATTRS_F32 MMS_SET_ATTRS_F32
#define S2M_SET_ATTRS_S32 MMS_SET_ATTRS_S32
#define S2M_GET_TIME ser_get_time

// Type declarations

/** Main ser2mms object structure. */
typedef struct ser2mms_s ser2mms_t;

/** Type alias for brevity. */
typedef ser2mms_t s2m_t;

// Public interface function declarations

// Basic functions

/**
* Object constructor.
* Creates and initializes new ser2mms object instance, allocates memory and
* configures transport layer.
*
* @param ied IED server instance
* @param mode operation mode (S2M_SLAVE or S2M_POLL)
* @param id device identifier
* @param stty_init pointer to transport initialization structure
* @return if object created - pointer to object,
*         if error occurred - NULL
*/
s2m_t *ser2mms_new(void *ied, u32_t mode, u32_t id, void *stty_init);

/**
* Object destructor.
* Releases all ser2mms object resources, stops threads (if
* used) and destroys transport layer.
*
* @param self pointer to object
*/
void ser2mms_destroy(s2m_t *);

/**
* Start operation.
* In multithreaded mode creates worker thread, in single-threaded - just
* starts transport layer.
*
* @param self pointer to object
* @return 0 on success, -1 on error
*/
s32_t ser2mms_run(s2m_t *);

/**
* Polling function.
* Polls transport layer input buffer for new received bytes.
* Intended for periodic calls from main program loop when thread
* support is disabled
*
* @param self pointer to object
*/
void ser2mms_poll(s2m_t *);

// Functions with external implementation

/** For SLAVE mode. */

/**
* Read command argument array.
* Function must be implemented by user
*
* @param[in] buf buffer with page data
* @param[in] ds dataset index
* @param[in] page data page number
* @param[in] opaque opaque context pointer
*/
void ser2mms_read_page(const page_prm_t *buf, u8_t ds, u8_t page, void *opaque);

/**
* Read subscription array.
* Function must be implemented by user
*
* @param[in] buf buffer with subscription data
* @param[in] opaque opaque context pointer
*/
void ser2mms_read_subs(const sub_prm_t *buf, void *opaque);

/**
* Write answer.
* Function must be implemented by user
*
* @param[out] buf answer buffer
* @param[out] buf_len pointer to buffer size location
*/
void ser2mms_write_answer(answ_prm_t *buf, u32_t *buf_len);

/**
* Set system time.
* Function must be implemented by user
*
* @param[out] epoch pointer to place time in UNIX epoch format (seconds)
* @param[out] usec pointer to place microsecond part of time
*/
void ser2mms_set_time(uint32_t *epoch, uint32_t *usec);

/** For POLL mode. */

/**
* Write command argument array.
* Function must be implemented by user
*
* @param[out] buf buffer with page data
* @param[out] buf_len pointer to buffer size location
* @param[in] ds dataset index
* @param[in] page data page number
*/
void ser2mms_write_page(page_prm_t *buf, u32_t *buf_len, u8_t ds, u8_t page);

/**
* Write subscription array.
* Function must be implemented by user
*
* @param[out] buf buffer with subscription data
* @param[out] buf_len pointer to buffer size location
*/
void ser2mms_write_subs(sub_prm_t *buf, u32_t *buf_len);

/**
* Read answer.
* Function must be implemented by user
*
* @param[in] buf answer buffer
*/
// void ser2mms_read_answer(answ_prm_t *buf);

// Helper functions

/**
* IED server instance getter.
*
* @param self pointer to object
* @return opaque pointer to IED server object
*/
void *ser2mms_get_ied(s2m_t *);

/**
* Setter for transmitted command type (only in S2M_POLL mode).
*
* @param self pointer to object
* @param cmd new command
*/
void ser2mms_set_cmd(s2m_t *, u32_t);

/**
* Device ID setter.
*
* @param self pointer to object
* @param id new ID value
*/
void ser2mms_set_id(s2m_t *, u32_t);

/**
* Test tick.
* Used for debugging and testing. Performs one processing step
* at transport layer.
*
* @param self pointer to object
*/
void ser2mms_test_tick(s2m_t *);

#endif // SER2MMS_H
