/**
  * @file   port_rs485.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   14-November-2025
  */

#ifndef __unix__
#error "Should only be compiled under a unix system"
#endif

#include "port_rs485.h"
#include "port_rs485_init.h"
#include "port_alloc.h"
#include "port_thread.h"
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
#include "gpio.h"
#endif
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/serial.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>

// Макрос RS485_USE_STATIC должен быть выкл. т.к. библиотека 'c-periphery' 
// не поддерживает статичной аллокации
#if (RS485_USE_STATIC)
#error "Macro 'RS485_USE_STATIC' must be disabled if you're using 'c-periphery'"
#endif

typedef enum { DIR_IN=0, DIR_OUT } dir_t;

struct rs485_s {
  fd_t  fd;
  //char  dev_name[16];
  struct termios tio_old;
  
  bool  sta_ena_rx;
  u8_t  rcvd_buf[RCVD_BUF_SIZE];
  u32_t rcvd_pos;
  void (*fn_rcv)(void *, u32_t);
  void  *fn_pld;
  
  bool  sta_ena_tx;
  bool  sta_send_tx;
  bool  sta_wait_tx;
  u8_t  xmit_buf[XMIT_BUF_SIZE];
  u32_t xmit_size;
  fn_t  fn_xmt;

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  gpio_t *nre_de;
#endif
};

static bool  receive(fd_t, u8_t *, u32_t, u32_t *);
static bool  transmit(fd_t, const u8_t *, u32_t);
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
static s32_t nre_de_init(rs485_t, const char *, u32_t);
static s32_t nre_de_set(rs485_t, dir_t);
static void  nre_de_del(rs485_t);
#endif

PORT_STATIC_DECLARE(RS485, struct rs485_s);

// ============================= Публичные функции =============================

/**
  * @brief  Constructor of 'rs485_t' object
  * @param  port - ?
  * @param  fn_rcv - ?
  * @param  fn_xmt - ?
  * @param  fn_pld - ?
  * @return ?
  */
rs485_t rs485_new(void *init, rs485_fn_t *fn)
{
  rs485_init_t *pinit = (rs485_init_t *)init;
  assert(pinit && fn);
  
  PORT_ALLOC(RS485, struct rs485_s, self, return NULL);
  
  self->fn_rcv = fn->func_rcv;
  self->fn_xmt = fn->func_xmt;
  self->fn_pld = fn->pld;
  
  // check the name
  if (!pinit->device_path) {
    printf("[rs485_new] Dev name must be valid\n");
    goto exit_0;
  }
  // try open
  self->fd = open( pinit->device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (self->fd < 0) {
    printf("[rs485_new] Cann't open dev '%s'\n", pinit->device_path);
    goto exit_0;
  }

  struct serial_struct serial;
  if (ioctl(self->fd, TIOCGSERIAL, &serial) == 0) {
    serial.flags |= ASYNC_LOW_LATENCY;
    if (ioctl(self->fd, TIOCSSERIAL, &serial) < 0)
      perror("[rs485_new] ASYNC_LOW not configured");
  }
  
  // Сохраняем текущие настройки
  tcgetattr(self->fd, &self->tio_old);
  // Получаем текущую конфигурацию как базу
  struct termios tio;
  tcgetattr(self->fd, &tio);

  // Очищаем флаги
  tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tio.c_oflag &= ~OPOST;
  tio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tio.c_cflag &= ~(CSIZE | PARENB);
  // Устанавливаем нужные параметры
  tio.c_iflag |= IGNBRK | INPCK;
  tio.c_cflag |= CREAD | CLOCAL | CS8;
  cfsetispeed(&tio, B230400);
  cfsetospeed(&tio, B230400);
  // Таймауты для неблокирующего чтения
  tio.c_cc[VMIN] = 0;
  tio.c_cc[VTIME] = 0;

  // Применяем настройки
  tcflush(self->fd, TCIFLUSH);
  if (tcsetattr(self->fd, TCSANOW, &tio) < 0) {
    perror("Error setting attributes");
    goto exit_1;
  }

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  if (pinit->gpio_path) {
    int32_t rc = nre_de_init(self, pinit->gpio_path, pinit->gpio_pin);
    if (rc) {
      perror("Error while init nre_de");
      goto exit_1;
    }
  }
#endif
  
  return self;
  
exit_1:
  close(self->fd);
exit_0:
  PORT_FREE(RS485, self);
  return NULL;
}

/**
  * @brief  Destructor
  * @param  self - ?
  */
void rs485_del(rs485_t self)
{
  assert(self);

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  nre_de_del(self);
#endif
  if (self->fd != -1) {
    tcsetattr( self->fd, TCSANOW, &self->tio_old );
    close( self->fd );
    self->fd = -1;
  }
  PORT_FREE(RS485, self);
}

/**
  * @brief  ?
  * @param  self - ?
  * @param  ena_rx - ?
  * @param  ena_tx - ?
  */
void rs485_ena(rs485_t self, bool ena_rx, bool ena_tx)
{
  assert(ena_rx ^ ena_tx);
  
  self->sta_ena_rx = ena_rx;
  if (ena_rx) {
    tcflush( self->fd, TCIFLUSH );
    self->rcvd_pos = 0;
  }
  
  
  if (ena_tx) {
    self->sta_ena_tx = true;
    self->sta_send_tx = true;
    self->sta_wait_tx = false;
    self->xmit_size = 0;
  } else {
    self->sta_ena_tx = false;
    self->sta_send_tx = false;
    self->sta_wait_tx = false;
  }
}

void rs485_ena_wait(rs485_t self, __UNUSED bool wait_tx)
{
  assert(self);
  self->sta_send_tx = false;
  self->sta_wait_tx = true;
}

/**
  * @brief  ?
  * @param  self - ?
  * @param  byte - ?
  */
bool rs485_get(rs485_t self, u8_t *byte)
{
  if (self->rcvd_pos < RCVD_BUF_SIZE) {
    *byte = self->rcvd_buf[self->rcvd_pos++];
    return true;
  }
  return false;
}

/**
  * @brief  ?
  * @param  self - ?
  * @param  byte - ?
  */
bool rs485_put(rs485_t self, u8_t byte)
{
  assert(self);
  if (self->xmit_size < XMIT_BUF_SIZE) {
    self->xmit_buf[self->xmit_size++] = byte;
    return true;
  }
  return false;
}

/**
  * @brief  ?
  * @param  self - ?
  */
void rs485_poll_rx(rs485_t self)
{
  u32_t rcvd = 0;
  
  if (self->sta_ena_rx) {
    if ( !receive(self->fd, self->rcvd_buf, RCVD_BUF_SIZE, &rcvd) ) {
      // printf("[rs485_poll_rx] Nothing to read\n");
      return;
    }
    if (rcvd > 0) {
      if (self->fn_rcv) self->fn_rcv(self->fn_pld, rcvd);
      self->rcvd_pos = 0;
    }
  }
}

/**
  * @brief  ?
  * @param  self - ?
  */
 void rs485_poll_tx(rs485_t self)
 {
   if (self->sta_ena_tx) {
     //
     if (self->sta_send_tx && !self->sta_wait_tx) {
       while (self->sta_send_tx) {
         if (self->fn_xmt) self->fn_xmt(self->fn_pld);
       }
 #if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
       if (self->nre_de) nre_de_set(self, DIR_OUT);
 #endif
       if (!transmit(self->fd, self->xmit_buf, self->xmit_size)) {
         perror("Сan't send the frame completely");
       }
 #if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
       thread_sleep(PORT_RS485_DE_WAIT);
       if (self->nre_de) nre_de_set(self, DIR_IN);
 #endif
     }
   }
 }

// ============================ Статические функции ============================

/**
  * @brief ?
  * @param self - ?
  * @param to_recv - ?
  */
static bool receive(fd_t fd, u8_t *buf, u32_t size, u32_t *rcvd)
{
  fd_set         rfds;
  struct timeval tv;
  ssize_t        rc;
  
  tv.tv_sec = 0;
  tv.tv_usec = 250; // 250 мкс
  FD_ZERO( &rfds );
  FD_SET( fd, &rfds );
  
  rc = select( fd + 1, &rfds, NULL, NULL, &tv );
  if (rc < 0) return false;
  
  if( !FD_ISSET(fd, &rfds) ) return false;
  
  rc = read( fd, (void *)buf, (size_t)size );
  if (rc < 0) return false;
  *rcvd = ( u32_t ) rc;
  
  return true;
}

/**
  * @brief ?
  * @param self - ?
  * @param to_send - ?
  */
static bool transmit(fd_t fd, const u8_t *buf, u32_t size)
{
  ssize_t res;
  size_t  left = ( size_t ) size;
  size_t  done = 0;

  while( left > 0 ) {
    if( ( res = write( fd, (const void *)buf + done, left ) ) == -1 ) {
      if( errno != EINTR ) break;
      /* call write again because of interrupted system call. */
      continue;
    }
    done += res;
    left -= res;
  }
  return left == 0 ? true : false;
}

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
/**
  * @brief Pin nRE/DE initializer
  */
static s32_t nre_de_init(rs485_t self, const char *path, u32_t line)
{
  assert(self);
  
  // try create
  if ( (self->nre_de = gpio_new()) == NULL ) {
    printf("Can't allocate 'gpio_t' object\n");
    return -1;
  }
  
  // try open
  if (gpio_open_sysfs(self->nre_de, line, GPIO_DIR_OUT) < 0) {
    printf( "Can't open 'gpio_t' object with path %s\n", path);
    goto exit_0;
  }
    
  // try set value
  if (nre_de_set(self, DIR_IN) < 0) {
    printf( "Can't set value to 'gpio_t' object\n");
    goto exit_1;
  }
  return 0;
  
  exit_1:
  gpio_close(self->nre_de);
  exit_0:
  gpio_free(self->nre_de);
  return -1;
}

/**
  * @brief Pin nRE/DE deinitializer
  */
static void nre_de_del(rs485_t self)
{
  assert(self);
  if (self->nre_de) {
    gpio_close(self->nre_de);
    gpio_free(self->nre_de);
    self->nre_de = NULL;
  }
}

/**
  * @brief Set the nRE/DE state (IN or OUT)
  */
static s32_t nre_de_set(rs485_t self, dir_t direction)
{
  if (!self) return -1;
  return gpio_write(self->nre_de, (direction==DIR_IN) ? false : true);
}

#endif
