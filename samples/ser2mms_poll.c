/**
* @file ser2mms_poll.c
* @author Ilia Proniashin, msg@proglyk.ru
* @date 09-October-2025
*
* Example implementation for ser2mms in POLL mode.
*/

#include "ser2mms.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void vSetSignal( int iSignalNr, void (*pSigHandler)(int) );
static void handler_sigquit(int sig);
static void handler_sigint(int sig);

int running = 1;
static s2m_t *s2m = NULL;

static rs485_init_t s2m_stty_init = {
#if (PORT_IMPL==PORT_IMPL_LINUX)
#if (LINUX_HW_IMPL==LINUX_HW_IMPL_WSL)
  .device_path = "/dev/ttyV0",
  // .device_path = "/dev/ttyUSB1",
  .gpio_path = NULL
#elif (LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  //#error "Not available"
  .device_path = NULL,
  .gpio_path = NULL,
#endif
#else
#error "Not available"
#endif
};

int main(void)
{
  char symb;
  vSetSignal(SIGQUIT, handler_sigquit); // code (3) 'Ctrl+\\'
  vSetSignal(SIGINT, handler_sigint);   // code (2) 'Ctrl+C'

  // init
  s2m = ser2mms_new(NULL, S2M_POLL, 12,
                    (void *)&s2m_stty_init);
  assert(s2m);

  // run
  ser2mms_run(s2m);
  printf("Runned\r\n");

  // loop
  do {
    printf( "> " );
    symb = getchar();
    switch (symb) {
      case 'a': printf("fuck you\r\n"); break;
      case 'q': running = 0; break;
    }
  } while( running );

  // close
  ser2mms_destroy(s2m);
  printf("Stopped\r\n");
  return 0;
}

/**
* Read page values
*/
void ser2mms_write_page(page_prm_t *buf, u32_t *buf_len, u8_t ds, u8_t page)
{
  u32_t cnt = 0;
  assert(buf && buf_len);
  // f32_t ftemp;
  // s32_t stemp;

  // Iterate over Dataset positions
  // Fill variables
  switch (ds)
  {
    // Dataset GGIO1
    case (1): {
      switch (page)
      {
        // Page 0
        case (0): {
          buf[cnt++].mag = 1;
          buf[cnt++].mag = 2;
          buf[cnt++].mag = 3;
          *buf_len = cnt;
        } break;

        // Page 1
        case (1): {
          buf[cnt++].mag = 4;
          buf[cnt++].mag = 5;
          buf[cnt++].mag = 6;
          *buf_len = cnt;
        } break;

        // Page 2
        case (2): {
          buf[cnt++].mag = 7;
          buf[cnt++].mag = 8;
          buf[cnt++].mag = 9;
          *buf_len = cnt;
        } break;

        // Page 3
        case (3): {
          buf[cnt++].mag = 10;
          buf[cnt++].mag = 11;
          buf[cnt++].mag = 12;
          *buf_len = cnt;
        } break;
      }
    } break;

    // Dataset GGIO2
    case (2): {
    } break;

    // Dataset GGIO3
    case (3): {
    } break;

    // Dataset GGIO4
    case (4): {
    } break;

    // Dataset GGIO5
    case (5): {
    } break;

    // Dataset GGIO6
    case (6): {
    } break;
  }
}

/**
* Read subscription values
*/
void ser2mms_write_subs(sub_prm_t *buf, u32_t *buf_len)
{
  uint32_t ts_int[2];
  assert(buf && buf_len);

#if (PORT_IMPL==PORT_IMPL_LINUX)
  struct timespec tspec;
  clock_gettime(CLOCK_REALTIME, &tspec);
  ts_int[0] = tspec.tv_sec;
  ts_int[1] = tspec.tv_nsec / 1000000;
#elif (PORT_IMPL==PORT_IMPL_ARM)
  GET_SYSTEM_TIME((timestamp+0),(timestamp+1));
#else
#error Macro 'PORT_IMPL' definition is needed
#endif

#if (!S2M_REDUCED)
  for (u32_t i=0; i<SER_NUM_SUBS; i++) {
    buf[i].mag = i;
    buf[i].t[0] = ts_int[0];
    buf[i].t[1] = ts_int[1];
  }
  *buf_len = SER_NUM_SUBS;
#else
  (void)buf;
  *buf_len = 0;
#endif
}

/**
* Write answer
*/
void ser2mms_write_answer(answ_prm_t *answ_buf, u32_t *answ_len)
{
  u32_t cnt = 0;
  f32_t ftemp = 100.0f;
  // DataAttribute *data_attr;
  // f32_t ftemp;
  // u16_t ustemp;

  // Iz
  // data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Iz_mag_f;
  // ftemp = MmsValue_toFloat(data_attr->mmsValue);
  ftemp *= 2.0f;
  answ_buf[cnt++].mag = (u16_t)((s32_t)ftemp & 0x0000ffff);

  // Rlz
  // data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Rlz_mag_f;
  // ftemp = MmsValue_toFloat(data_attr->mmsValue);
  ftemp *= 2.0f;
  answ_buf[cnt++].mag = (u16_t)((s32_t)ftemp & 0x0000ffff);

  // usCirc
  // data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Circ_mag_i;
  // self->outdata.usCirc = (u16_t)(MmsValue_toInt32(data_attr->mmsValue) &
  //                                 0x0000ffff);
  // answ_buf[cnt++] = (u16_t)( MmsValue_toInt32(data_attr->mmsValue) & 0x0000ffff );
  ftemp *= 2.0f;
  answ_buf[cnt++].mag = (u16_t)((s32_t)ftemp & 0x0000ffff);
  *answ_len = cnt;
}

static void vSetSignal( int iSignalNr, void (*pSigHandler)(int) )
{
  struct sigaction sa = {0};
  // Enable signal SIGUSR1 and set handler
  sa.sa_handler = pSigHandler;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = 0;
  sigaction(iSignalNr, &sa, NULL);
}

static void handler_sigquit(int sig)
{
  printf("[sig] SIGQUIT (%d)\r\n", sig);
  switch ( sig ) {
    case SIGQUIT:
      ser2mms_test_tick(s2m);
      break;
  }
}

static void handler_sigint(int sig)
{
  static u32_t mode = 0;
  static u32_t id = 1;
  printf("[sig] SIGINT (%d)\r\n", sig);
  switch ( sig ) {
    case SIGINT:
      //ser2mms_set_id(s2m, id++);
      ser2mms_set_cmd(s2m, mode ^= 1);
      //printf("[sig] mode=%01d\n", id);
      //mb__test_recv(mb__inst());
      break;
  }
}
