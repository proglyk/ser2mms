/**
 * @file port_pps.h
 * @brief 1PPS signal generator module for BeagleBone Black P9_18
 * @date 30-November-2025
 */

#ifndef PORT_PPS_H
#define PORT_PPS_H

#include "port_conf.h"
#include "port_types.h"

#define PPS_USE_STATIC                  (0)

typedef struct pps_s *pps_t;

/**
 * @brief Create new 1PPS generator instance
 * @return Pointer to 1PPS instance or NULL on error
 */
pps_t pps_new(void);

/**
 * @brief Destroy 1PPS generator instance
 * @param self Pointer to 1PPS instance
 */
void pps_destroy(pps_t self);

/**
 * @brief Start 1PPS signal generation in separate thread
 * @param self Pointer to 1PPS instance
 * @return 0 on success, negative on error
 */
s32_t pps_run(pps_t self);

/**
 * @brief Stop 1PPS signal generation
 * @param self Pointer to 1PPS instance
 */
void pps_stop(pps_t self);

#endif // PORT_PPS_H
