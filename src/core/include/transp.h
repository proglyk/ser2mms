/**
 * @file transp.h
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 30-September-2025
 * 
 * Transport layer interface.
 * Provides functions for working with the protocol transport layer,
 * including initialization, event polling, and receive/transmit data management.
 */

#ifndef SER2MMS_TRANSP_H
#define SER2MMS_TRANSP_H

#include "ser2mms_conf.h"
// #include "types.h"
#include "port_types.h"

/** Use static allocation. */
#define TRANSP_USE_STATIC (0)

/** Pointer type to transport layer object. */
typedef struct transp_s transp_t;

// Public interface function declarations

// Basic functions

/**
 * Transport layer object constructor.
 * Initializes the transport layer object with callbacks and configuration.
 * Creates RS485 interface, event handler, and serial protocol handler.
 * 
 * @param argc argument count (unused)
 * @param pdata pointer to data array (unused)
 * @param argv additional arguments (unused)
 * @param irq interrupt context (unused)
 * @param pld_api pointer to payload API context
 * @param mode operation mode (MODE_POLL or MODE_SLAVE)
 * @param id device address identifier
 * @param stty_init pointer to RS485 initialization structure
 * @return pointer to transport object on success, NULL on allocation or initialization error
 */
void *transp_new(int argc, int *pdata, void *argv, void *irq,
                 void *pld_api, u32_t mode, u32_t id, void *stty_init);

/**
 * Transport layer object destructor.
 * Destroys the transport object and frees all resources.
 * 
 * @param argc argument count (unused)
 * @param opaque pointer to transport layer object
 */
void transp_destroy(int argc, void *opaque);

/**
 * Start transport layer operation.
 * Puts the transport layer into operational state and enables data reception.
 * 
 * @param self pointer to object
 */
void transp_run(transp_t *self);

/**
 * Poll transport layer.
 * Polls the transport layer for events and processes receive and transmit
 * state machines depending on the operation mode.
 * 
 * @param self pointer to object
 * @return 0 on success
 */
int transp_poll(transp_t *self);

// Helper functions

/**
 * Timer interrupt entry point.
 * Timer event handler for initiating data transmission in polling mode.
 * 
 * @param self pointer to object
 */
void transp_tick(transp_t *self);

/**
 * UART receive interrupt entry point.
 * Interrupt handler for receiving data via serial interface.
 * 
 * @param self pointer to object
 */
void transp_recv(transp_t *self);

/**
 * UART transmit interrupt entry point.
 * Interrupt handler for transmitting data via serial interface.
 * 
 * @param self pointer to object
 */
void transp_xmit(transp_t *self);

/**
 * Getter for 'ser' module pointer.
 * 
 * @param self pointer to object
 * @return pointer to serial protocol handler
 */
void *transp_get_top(transp_t *self);

/**
 * Device ID setter.
 * 
 * @param self pointer to object
 * @param id new device address identifier
 */
void transp_set_id(transp_t *self, u32_t id);

#endif
