/**
* @file ser2mms_slave.c
* @author Ilia Proniashin, msg@proglyk.ru
* @date 09-October-2025
*
* Example implementation for ser2mms in SLAVE mode.
*/

#include "ser2mms.h"
#include "ser2mms_defs.h"

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
#include "gpio.h"
#include "port_thread.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define P9_23                           (17 + 32*1) // uart2_de

static void vSetSignal( int iSignalNr, void (*pSigHandler)(int) );
static void handler_sigterm(int sig);
static void handler_sigint(int sig);

int running = 1;
static s2m_t *s2m = NULL;

static rs485_init_t s2m_stty_init = {
#if (PORT_IMPL==PORT_IMPL_LINUX)
#if (LINUX_HW_IMPL==LINUX_HW_IMPL_WSL)
  .device_path = "/dev/ttyUSB0",
  //.device_path = "/dev/ttyV1",
  .gpio_path = NULL
#elif (LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  .device_path = "/dev/ttyS2", // uart2, pins P9_21, P9_22
  .gpio_path = "/dev/gpiochip1",
  .gpio_pin = P9_23
#endif
#else
#error "Not available"
#endif
};


int main(void)
{
  // code (15) "sudo systemctl gololed stop"
  vSetSignal(SIGTERM, handler_sigterm);
  // code (2) 'Ctrl+C'
  vSetSignal(SIGINT, handler_sigint);

  // init
  s2m_t *s2m = ser2mms_new(
    NULL,                   // MMS stack
    S2M_SLAVE,              // Mode (SLAVE or POLL)
    12,                     // Address
    (void *)&s2m_stty_init  // Configuration
  );
  if (!s2m) {
    perror("Can't create s2m instance");
    exit(1);
  }

  // run
  ser2mms_run(s2m);

  // loop
  do {
    sleep(1);
  } while( running );

  // close
  ser2mms_destroy(s2m);
  return 0;
}

/**
* Read page values
*/
void ser2mms_read_page(const page_prm_t *buf, u8_t ds, u8_t page, void *opaque)
{
  (void)ds; (void)page; (void)buf; (void)opaque;
#if (!S2M_USE_LIBIEC)
  // printf("[write_carg_slave] Value #1: %02d\r\n", carg_buf[ 0]);
  // printf("[write_carg_slave] Value #1: %02d\r\n", carg_buf[ 1]);
  // printf("[write_carg_slave] Value #1: %02d\r\n", carg_buf[ 2]);
  void *ied = ser2mms_get_ied((s2m_t *)opaque);
  S2M_SET_PARAMS_S32(GGIO0, ConnStatus, HV, LV, buf);
#endif // S2M_USE_LIBIEC
}

/**
* Read subscription values
*/
void ser2mms_read_subs(const sub_prm_t *buf, void *opaque)
{
  (void)opaque; (void)buf;
#if (!S2M_USE_LIBIEC)
  // printf("[write_subs_slave] Value #1: %02d\r\n", subs_buf[ 0].sl);
  // printf("[write_subs_slave] Epoch #1: %010d\r\n", subs_buf[ 0].pul[0]);
  // printf("[write_subs_slave] Usec #1: %d\r\n", subs_buf[ 0].pul[1]);
  void *ied = ser2mms_get_ied((s2m_t *)opaque);
  S2M_SET_ATTRS_S32(GGIO0, ConnStatus, buf[0].mag, buf[0].t, true);
#endif // S2M_USE_LIBIEC
}

/**
* Write answer
*/
void ser2mms_write_answer(answ_prm_t *answ_buf, u32_t *answ_len)
{
  u32_t cnt = 0;
  // (void)answ_buf;
  // (void)answ_len;
  // #if (S2M_USE_LIBIEC==1)
#if (0)
  DataAttribute *data_attr;
  f32_t ftemp;
  u16_t ustemp;

  // Iz
  data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Iz_mag_f;
  ftemp = MmsValue_toFloat(data_attr->mmsValue);
  answ_buf[cnt++] = (u16_t)((s32_t)ftemp & 0x0000ffff);

  // Rlz
  data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Rlz_mag_f;
  ftemp = MmsValue_toFloat(data_attr->mmsValue);
  answ_buf[cnt++] = (u16_t)((s32_t)ftemp & 0x0000ffff);

  // usCirc
  data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Circ_mag_i;
  answ_buf[cnt++] = (u16_t)( MmsValue_toInt32(data_attr->mmsValue) & 0x0000ffff );
  *answ_len = cnt;
#else
  answ_buf[cnt++].mag = 1;
  answ_buf[cnt++].mag = 2;
  answ_buf[cnt++].mag = 3;
  *answ_len = cnt;
#endif
}

/**
* Set system time
*/
void ser2mms_set_time(uint32_t *epoch, uint32_t *usec)
{
  assert(epoch && usec);
#if (PORT_IMPL==PORT_IMPL_LINUX)
  struct timespec tspec;
  clock_gettime(CLOCK_REALTIME, &tspec);
  *epoch = tspec.tv_sec;
  *usec = tspec.tv_nsec / 1000000;
#elif (PORT_IMPL==PORT_IMPL_ARM)
  GET_SYSTEM_TIME((timestamp+0),(timestamp+1));
#else
#error Macro 'PORT_IMPL' definition is needed
#endif
}

/**
* Signal handler setup
*/
static void vSetSignal( int iSignalNr, void (*pSigHandler)(int) )
{
  struct sigaction sa = {0};
  // Enable signal SIGUSR1 and set handler
  sa.sa_handler = pSigHandler;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = 0;
  sigaction(iSignalNr, &sa, NULL);
}

/**
* SIGTERM handler
*/
static void handler_sigterm(int sig)
{
  switch ( sig ) {
    case SIGTERM: running = 0; break;
  }
}

/**
* SIGINT handler
*/
static void handler_sigint(int sig)
{
  switch ( sig ) {
    case SIGINT: running = 0; break;
  }
}
