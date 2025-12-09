/**
 * @file port_can.c
 * @brief CAN bus wrapper module implementation
 * @date 28-November-2025
 */

#include "port_can.h"
#include "port_alloc.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <poll.h>
#include <errno.h>

// #if (!PORT_USE_CAN)
// #error "Macro 'PORT_USE_CAN' must be enabled if you're using port_can.c"
// #endif

struct can_s
{
  int  fd;          // Socket file descriptor
  int  ifindex;     // Interface index
  char ifname[16];  // Interface name
  int  state;       // Connection state
};

PORT_STATIC_DECLARE(CAN, struct can_s);

/**
 * @brief Open CAN interface
 */
can_t can_new(const char *ifname, u32_t flags)
{
  PORT_ALLOC(CAN, struct can_s, self, return NULL);
  
  struct sockaddr_can addr;
  struct ifreq ifr;
  
  // Create socket
  self->fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (self->fd < 0) {
    goto exit;
  }
  
  // Get interface index
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
  ifr.ifr_name[IFNAMSIZ - 1] = '\0';
  
  if (ioctl(self->fd, SIOCGIFINDEX, &ifr) < 0) {
    close(self->fd);
    goto exit;
  }
  
  // Setup address
  memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  
  // Bind socket to interface
  if (bind(self->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(self->fd);
    goto exit;
  }
  
  // Store interface info
  self->ifindex = ifr.ifr_ifindex;
  strncpy(self->ifname, ifname, sizeof(self->ifname) - 1);
  self->state = 1;
  
  return self;
    
exit:
  PORT_FREE(CAN, self);
  return NULL;
}

/**
 * @brief Close CAN interface
 */
void can_del(can_t self)
{
  assert(self);
  
  if (self->fd >= 0) {
    close(self->fd);
  }
  
  PORT_FREE(CAN, self);
}

/**
 * @brief Send CAN frame
 */
s32_t can_write(can_t self, const can_frame_t *frame)
{
  assert(self);
  assert(frame);
  
  struct can_frame kernel_frame;
  
  // Convert to kernel format
  kernel_frame.can_id = frame->id;
  kernel_frame.can_dlc = frame->dlc;
  memcpy(kernel_frame.data, frame->data, frame->dlc);
  
  s32_t nbytes = write(self->fd, &kernel_frame, sizeof(kernel_frame));
  
  return (nbytes == sizeof(kernel_frame)) ? nbytes : -errno;
}

/**
 * @brief Receive CAN frame
 */
s32_t can_read(can_t self, can_frame_t *frame, s32_t timeout_ms)
{
  assert(self);
  assert(frame);
  
  struct can_frame kernel_frame;
  struct pollfd pfd;
  
  // Setup poll for timeout handling
  if (timeout_ms >= 0) {
    pfd.fd = self->fd;
    pfd.events = POLLIN;
    
    int ret = poll(&pfd, 1, timeout_ms);
    if (ret < 0) {
        return -errno;
    }
    if (ret == 0) {
        return -ETIMEDOUT;  // Timeout
    }
  }
  
  // Read frame
  s32_t nbytes = read(self->fd, &kernel_frame, sizeof(kernel_frame));
  
  if (nbytes == sizeof(kernel_frame)) {
    // Convert from kernel format
    frame->id = kernel_frame.can_id & CAN_EFF_MASK;
    frame->dlc = kernel_frame.can_dlc;
    memcpy(frame->data, kernel_frame.data, kernel_frame.can_dlc);
    frame->flags = 0;
    
    if (kernel_frame.can_id & CAN_EFF_FLAG) {
        frame->flags |= 0x01;  // Extended frame
    }
    if (kernel_frame.can_id & CAN_RTR_FLAG) {
        frame->flags |= 0x02;  // RTR frame
    }
    
    return nbytes;
  }
  
  return (nbytes < 0) ? -errno : -EIO;
}

/**
 * @brief Set CAN filters
 */
s32_t can_set_filter(can_t self, u32_t id, u32_t mask)
{
  assert(self);
  
  struct can_filter rfilter;
  
  rfilter.can_id = id;
  rfilter.can_mask = mask;
  
  int rc = setsockopt(self->fd, SOL_CAN_RAW, CAN_RAW_FILTER,
                      &rfilter, sizeof(rfilter));
  
  return (rc < 0) ? -errno : 0;
}

/**
 * @brief Get file descriptor
 */
s32_t can_get_fd(can_t self)
{
  assert(self);
  return self->fd;
}
