/**
  * @file   ser2mms_pool.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   09-October-2025
  */

#include "ser2mms.h"
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>


static void write_carg_poll( void *, u16_t *, u32_t, u8_t, u8_t );
static void write_subs_poll( void *, prm_t *, u32_t );
static void read_answ_poll( void *, u16_t *, u32_t * );
static void vSetSignal( int iSignalNr, void (*pSigHandler)(int) );
static void handler_sigquit(int sig);
static void handler_sigint(int sig);

volatile bool runned = true;
static s2m_t *s2m = NULL;

static rs485_init_t s2m_stty_init = {
#if (PORT_IMPL==PORT_IMPL_LINUX)
#if (LINUX_HW_IMPL==LINUX_HW_IMPL_WSL)
  .device_path = "/dev/ttyV0",
//.device_path = "/dev/ttyUSB0",
  .gpio_path   = NULL
#elif (LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
#error "Not available"
#endif
#else
#error "Not available"
#endif
};

int main(void)
{
  char symb;
  
  vSetSignal(SIGQUIT, handler_sigquit); // код (3) 'Ctrl+\'
  vSetSignal(SIGINT,  handler_sigint);  // код (2) 'Ctrl+C'
  
  // init
  s2m = ser2mms_new(NULL, write_carg_poll, write_subs_poll, read_answ_poll, 
                    S2M_POLL, 12, 
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
      case 'q': runned = false; break;
    }
  } while( runned );
  
  // close
  ser2mms_stop(s2m);
  printf("Stopped\r\n");
  return 0; 
}

/**
  * @brief Функция обновления полей датасетов.
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  * @retval none: Нет
  */
static void write_carg_poll( void *opaque, u16_t *carg_buf, 
                        __UNUSED u32_t carg_len, u8_t ds, u8_t page )
{
  // f32_t ftemp;
  // s32_t stemp;
  s2m_t *s2m = (s2m_t *)opaque;
  assert(s2m);
  
  // Пробегаемся по позициям Датасетов
  // заполняем переменные
  switch (ds)
  {    
    // Датасет GGIO1
     case (1): {
      switch (page)
      {
        // Страница 0
        case (0): {
          carg_buf[0] = 1;
          carg_buf[1] = 2;
          carg_buf[2] = 3;
        } break;
        
        // Страница 1
        case (1): {
          carg_buf[0] = 4;
          carg_buf[1] = 5;
          carg_buf[2] = 6;
        } break;
        
        // Страница 2
        case (2): {
          carg_buf[0] = 7;
          carg_buf[1] = 8;
          carg_buf[2] = 9;
        } break;
        
        // Страница 3
        case (3): {
          carg_buf[0] = 10;
          carg_buf[1] = 11;
          carg_buf[2] = 12;
        } break;
      }
    } break;
    
    // Датасет GGIO2
    case (2): {
      
    }  break;
    
    // Датасет GGIO3
    case (3): {
      
    } break;
    
    // Датасет GGIO4
    case (4): {
      
    } break;
    
    // Датасет GGIO5
    case (5): {
    
    } break;
    
    // Датасет GGIO6
    case (6): {
      
    } break;
  }
}

/**  ----------------------------------------------------------------------------
  * @brief Функция записи полей подписок
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  * @retval none: Нет */
static void write_subs_poll(void *opaque, prm_t *subs_buf, 
                            u32_t subs_len)
{
  uint32_t ts_int[2];
  
  assert(subs_buf);
  (void)opaque;
  (void)subs_len;

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
    
  subs_buf[0].sl = 1;
  subs_buf[0].pul[0] = ts_int[0];
  subs_buf[0].pul[1] = ts_int[1];
  
  subs_buf[1].sl = 2;
  subs_buf[1].pul[0] = 1762506309;
  subs_buf[1].pul[1] = 100;
  
  subs_buf[2].sl = 3;
  subs_buf[2].pul[0] = 1762506317;
  subs_buf[2].pul[1] = 150;
  
  subs_buf[3].sl = 4;
  subs_buf[3].pul[0] = 1762506324;
  subs_buf[3].pul[1] = 200;
  
  subs_buf[4].sl = 5;
  subs_buf[4].pul[0] = 1762506327;
  subs_buf[4].pul[1] = 250;
  
  subs_buf[5].sl = 6;
  subs_buf[5].pul[0] = 1762506340;
  subs_buf[5].pul[1] = 300;
  
  subs_buf[6].sl = 7;
  subs_buf[6].pul[0] = 1762506345;
  subs_buf[6].pul[1] = 350;
  
  subs_buf[7].sl = 8;
  subs_buf[7].pul[0] = 1762506351;
  subs_buf[7].pul[1] = 400;
  
  subs_buf[8].sl = 9;
  subs_buf[8].pul[0] = 1762506355;
  subs_buf[8].pul[1] = 450;
  
  subs_buf[9].sl = 10;
  subs_buf[9].pul[0] = 1762506360;
  subs_buf[9].pul[1] = 500;
  
  subs_buf[10].sl = 11;
  subs_buf[10].pul[0] = 1762506365;
  subs_buf[10].pul[1] = 550;
  
}

/**  ----------------------------------------------------------------------------
  * @brief Функция обновления полей датасетов.
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  * @retval none: Нет */
static void read_answ_poll( __UNUSED void *argv, u16_t *answ_buf,
                            __UNUSED u32_t *answ_len )
{
  // DataAttribute *data_attr;
  f32_t ftemp = 1.0f;
  u32_t cnt = 0;
  
  // Iz
  // data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Iz_mag_f;
  // ftemp = MmsValue_toFloat(data_attr->mmsValue);
  ftemp *= 2.0f;
  answ_buf[cnt++] = (u16_t)((s32_t)ftemp & 0x0000ffff);

  // Rlz
  // data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Rlz_mag_f;
  // ftemp = MmsValue_toFloat(data_attr->mmsValue);
  ftemp *= 2.0f;
  answ_buf[cnt++] = (u16_t)((s32_t)ftemp & 0x0000ffff);

  // usCirc
  // data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Circ_mag_i;
  // self->outdata.usCirc = (u16_t)(MmsValue_toInt32(data_attr->mmsValue) & 
    // 0x0000ffff);
  // answ_buf[cnt++] = (u16_t)( MmsValue_toInt32(data_attr->mmsValue) & 0x0000ffff );
  ftemp *= 2.0f;
  answ_buf[cnt++] = (u16_t)((s32_t)ftemp & 0x0000ffff);
  //*answ_len = cnt;
}

static void vSetSignal( int iSignalNr, void (*pSigHandler)(int) )
{
  struct sigaction sa = {0};
  // Разрешаем сигнал SIGUSR1 и ставим обработчик
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
      ser2mms_set_id(s2m, id++);
      // s2m_set_cmd(s2m, mode ^= 1);
      //printf("[sig] mode=%01d\n", id);
      //mb__test_recv(mb__inst());
    break;
  }
}
