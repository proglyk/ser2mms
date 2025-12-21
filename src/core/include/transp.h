/**
* @file transp.h
* @author Ilia Proniashin, msg@proglyk.ru
* @date 30-September-2025
*/

#ifndef SER2MMS_TRANSP_H
#define SER2MMS_TRANSP_H

#include "ser2mms_conf.h"
#include "types.h"
#include "port_types.h"

// Allocation
#define TRANSP_USE_STATIC               (0) //S2M_USE_STATIC

typedef struct transp_s transp_t;

/**
* @brief Polls the transport layer for events and processes state machines
* @param self Pointer to this instance
* @retval 0 on success
*/
int transp_poll(transp_t *self);

/**
* @brief Initializes transport layer instance with callbacks and configuration
* @param argc Argument count (unused)
* @param pdata Pointer to data array (unused)
* @param argv Additional arguments (unused)
* @param irq Interrupt context (unused)
* @param fn1 Callback function for updating parameters/data
* @param fn2 Callback function for updating subscription parameters
* @param fn3 Callback function for generating response
* @param pld_api Pointer to payload API context
* @param mode Operating mode (MODE_POLL or MODE_SLAVE)
* @param id Device address identifier
* @param stty_init Pointer to RS485 initialization structure
* @retval Pointer to transport instance on success
* @retval NULL on allocation or initialization failure
*/
void *transp_new(int argc, int *pdata, void *argv, void *irq, 
                 void *pld_api, u32_t mode, u32_t id, void *stty_init);

/**
* @brief Starts transport layer operation
* @param self Pointer to this instance
*/
void transp_run(transp_t *self);

/**
* @brief Destroys transport instance and releases all resources
* @param argc Argument count (unused)
* @param opaque Pointer to transport instance
*/
void transp_destroy(int argc, void *opaque);

/**
* @brief Returns pointer to timer handle
* @param self Pointer to this instance
* @retval Pointer to timer instance
*/
void *transp_get_tim_h(transp_t *self);

/**
* @brief Returns pointer to serial protocol handler
* @param self Pointer to this instance
* @retval Pointer to serial handler instance
*/
void *transp_get_ser_h(transp_t *self);

/**
* @brief UART receive interrupt entry point
* @param self Pointer to this instance
*/
void transp_recv(transp_t *self);

/**
* @brief UART transmit interrupt entry point
* @param self Pointer to this instance
*/
void transp_xmit(transp_t *self);

/**
* @brief Timer interrupt entry point
* @param self Pointer to this instance
*/
void transp_tick(transp_t *self);

/**
* @brief Returns pointer to top layer (serial protocol handler)
* @param self Pointer to this instance
* @retval Pointer to serial protocol handler
*/
void *transp_get_top(transp_t *self);

/**
* @brief Sets device address identifier
* @param self Pointer to this instance
* @param id New device address identifier
*/
void transp_set_id(transp_t *self, u32_t id);

//TEST
void mb_tp__tick(void*);

#endif
