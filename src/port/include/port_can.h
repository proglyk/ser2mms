/**
 * @file port_can.h
 * @brief CAN bus wrapper module
 * @date 28-November-2025
 */

#ifndef PORT_CAN_H
#define PORT_CAN_H

#include "port_types.h"

#define CAN_USE_STATIC                  (0) //PORT_USE_STATIC

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Opaque CAN handle type
 */
typedef struct can_s *can_t;

/**
 * @brief CAN frame structure
 */
typedef struct can_frame_s
{
  u32_t id;       // CAN ID (11 or 29 bit)
  u8_t  dlc;      // Data Length Code (0-8)
  u8_t  data[8];  // Data payload
  u8_t  flags;    // Flags (RTR, EFF, ERR)
} can_frame_t;

/**
 * @brief Open CAN interface
 * @param ifname Interface name (e.g., "can0")
 * @param flags Configuration flags (bitrate, mode, etc.)
 * @return CAN handle or NULL on error
 */
can_t can_new(const char *ifname, u32_t flags);

/**
 * @brief Close CAN interface
 * @param self CAN handle
 */
void can_del(can_t self);

/**
 * @brief Send CAN frame
 * @param self CAN handle
 * @param frame Pointer to CAN frame
 * @return Number of bytes sent or negative error code
 */
s32_t can_write(can_t self, const can_frame_t *frame);

/**
 * @brief Receive CAN frame
 * @param self CAN handle
 * @param frame Pointer to buffer for received frame
 * @param timeout_ms Timeout in milliseconds (0 = non-blocking, -1 = blocking)
 * @return Number of bytes received or negative error code
 */
s32_t can_read(can_t self, can_frame_t *frame, s32_t timeout_ms);

/**
 * @brief Set CAN filters
 * @param self CAN handle
 * @param id CAN ID to filter
 * @param mask CAN ID mask
 * @return 0 on success, negative error code on failure
 */
s32_t can_set_filter(can_t self, u32_t id, u32_t mask);

/**
 * @brief Get file descriptor for select/poll
 * @param self CAN handle
 * @return File descriptor or negative error code
 */
s32_t can_get_fd(can_t self);

#if defined(__cplusplus)
}
#endif

#endif /* PORT_CAN_H */
