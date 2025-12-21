/**
  * @file   ser2mms_slave.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   09-October-2025
  */

#include "ser2mms.h"
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
#include "gpio.h"
#include "port_thread.h"
#endif
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


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
  .gpio_path   = NULL
#elif (LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  .device_path = "/dev/ttyS2",  // uart2, ноги P9_21, P9_22
  .gpio_path   = "/dev/gpiochip1",
  .gpio_pin    = P9_23
#endif
#else
#error "Not available"
#endif
};
extern int s2m_toggle;
static int toggle = 0;

int main(void)
{
  // код (15) "sudo systemctl gololed stop"
  vSetSignal(SIGTERM, handler_sigterm);
  // код (2) 'Ctrl+C'  
  vSetSignal(SIGINT,  handler_sigint); 
  
  // init
  s2m_t *s2m = ser2mms_new(
    NULL,                   // MMS stack
    S2M_SLAVE,              // Mode (SLAVE or POLL)
    12,                     // Address 
    (void *)&s2m_stty_init  // Настройка
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
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
#endif
  ser2mms_stop(s2m);
  return 0; 
}

/**
  * @brief Чтение значений страниц
  */
void ser2mms_read_carg(void *opaque, u16_t *carg_buf, __UNUSED u32_t carg_len, u8_t ds, u8_t page)
{
  float ftemp;
  s32_t stemp;

// #if (S2M_USE_LIBIEC==1)
#if (0)
  void *ied = ser2mms_get_ied((s2m_t *)opaque);
  if (!ied) {
    printf("[write_carg] Ptr to ied is null\n");
    return;
  }
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
          // Параметр 0
          ftemp = (f32_t)carg_buf[0];
          S2M_SET_ATTR(f32, ied, IEDMODEL_UPG_GGIO1_Id_mag_f, ftemp);
          S2M_SET_ATTR(t,   ied, IEDMODEL_UPG_GGIO1_Id_t,     NULL);
          S2M_SET_ATTR(q,   ied, IEDMODEL_UPG_GGIO1_Id_q,     true);
          // Параметр 1
          ftemp = (f32_t)carg_buf[1];
          S2M_SET_ATTR(f32, ied, IEDMODEL_UPG_GGIO1_Ud_mag_f, ftemp);
          S2M_SET_ATTR(t,   ied, IEDMODEL_UPG_GGIO1_Ud_t,     NULL);
          S2M_SET_ATTR(q,   ied, IEDMODEL_UPG_GGIO1_Ud_q,     true);
          // Параметр 2
          ftemp = (f32_t)carg_buf[2];
          S2M_SET_ATTR(f32, ied, IEDMODEL_UPG_GGIO1_Iz_mag_f, ftemp);
          S2M_SET_ATTR(t,   ied, IEDMODEL_UPG_GGIO1_Iz_t,     NULL);
          S2M_SET_ATTR(q,   ied, IEDMODEL_UPG_GGIO1_Iz_q,     true);
        } break;
        
        // Страница 1
        case (1): {
/*           // Параметр 0
          stemp = (s32_t)carg_buf[0];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_PKZ_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_PKZ_t, carg_buf[0].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_PKZ_q, true);
          }
          // Параметр 1
          stemp = (s32_t)carg_buf[1];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_POS_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_POS_t, carg_buf[1].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_POS_q, true);
          }
          // Параметр 2
          stemp = (s32_t)carg_buf[2];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_SMU_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_SMU_t, carg_buf[2].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_SMU_q, true);
          } */
        } break;
        
        // Страница 2
        case (2): {
/*           // Параметр 0
          stemp = (s32_t)carg_buf[0];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_TAminus_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_TAminus_t, carg_buf[0].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_TAminus_q, true);
          }
          // Параметр 1
          stemp = (s32_t)carg_buf[1];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_TAplus_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_TAplus_t, carg_buf[1].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_TAplus_q, true);
          }
          // Параметр 2
          stemp = (s32_t)carg_buf[2];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_TBminus_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_TBminus_t, carg_buf[2].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_TBminus_q, true);
          } */
        } break;
        
        // Страница 3
        case (3): {
/*           // Параметр 0
          stemp = (s32_t)carg_buf[0];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_TBplus_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_TBplus_t, carg_buf[0].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_TBplus_q, true);
          }
          // Параметр 1
          stemp = (s32_t)carg_buf[1];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_TCminus_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_TCminus_t, carg_buf[1].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_TCminus_q, true);
          }
          // Параметр 2
          stemp = (s32_t)carg_buf[2];
          if ( SET_ATTR(s32, p, IEDMODEL_UPG_GGIO1_TCplus_mag_i, stemp) ) {
            //SET_ATTR(t, X, IEDMODEL_UPG_GGIO1_TCplus_t, carg_buf[2].pul);
            SET_ATTR(q, p, IEDMODEL_UPG_GGIO1_TCplus_q, true);
          } */
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
#else
  (void)ftemp;
  (void)stemp;
  (void)carg_buf;
  (void)ds;
  (void)page;

  // printf("[write_carg_slave] Value #1: %02d\r\n",  carg_buf[ 0]);
  // printf("[write_carg_slave] Value #1: %02d\r\n",  carg_buf[ 1]);
  // printf("[write_carg_slave] Value #1: %02d\r\n",  carg_buf[ 2]);

#endif // (S2M_USE_LIBIEC==1)
}

/**
  * @brief Чтение значений подписок
  */
void ser2mms_read_subs(void *opaque, prm_t *subs_buf, __UNUSED u32_t subs_len)
{
  (void)opaque;
  (void)subs_buf;
  
  // printf("[write_subs_slave] Value #1: %02d\r\n",  subs_buf[ 0].sl);
  // printf("[write_subs_slave] Epoch #1: %010d\r\n", subs_buf[ 0].pul[0]);
  // printf("[write_subs_slave] Usec  #1: %d\r\n",  subs_buf[ 0].pul[1]);
  
  // printf("[write_subs_slave] Epoch #2: %010d\r\n", subs_buf[ 1].pul[0]);
  // printf("[write_subs_slave] Epoch #3: %010d\r\n", subs_buf[ 2].pul[0]);
  // printf("[write_subs_slave] Epoch #4: %010d\r\n", subs_buf[ 3].pul[0]);
  // printf("[write_subs_slave] Epoch #5: %010d\r\n", subs_buf[ 4].pul[0]);
  // printf("[write_subs_slave] Epoch #6: %010d\r\n", subs_buf[ 5].pul[0]);
  // printf("[write_subs_slave] Epoch #7: %010d\r\n", subs_buf[ 6].pul[0]);
  // printf("[write_subs_slave] Epoch #8: %010d\r\n", subs_buf[ 7].pul[0]);
  // printf("[write_subs_slave] Epoch #9: %010d\r\n", subs_buf[ 8].pul[0]);
  // printf("[write_subs_slave] Epoch#10: %010d\r\n", subs_buf[ 9].pul[0]);
  // printf("[write_subs_slave] Epoch#11: %010d\r\n", subs_buf[10].pul[0]);
}

/**
  * @brief запись ответа
  */
void ser2mms_write_answer(void *argv, u16_t *answ_buf, u32_t *answ_len)
{
  u32_t cnt = 0;
  (void)argv;
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
  answ_buf[cnt++] = 1;
  answ_buf[cnt++] = 2;
  answ_buf[cnt++] = 3;
  *answ_len = cnt;
#endif
}

/**
  * @brief Установка времени
  */
void ser2mms_set_time(uint32_t *epoch, uint32_t *usec)
{
  assert(epoch && usec);
#if (PORT_IMPL==PORT_IMPL_LINUX)
  struct timespec tspec;
  clock_gettime(CLOCK_REALTIME, &tspec);
  *epoch = tspec.tv_sec;
  *usec  = tspec.tv_nsec / 1000000;
#elif (PORT_IMPL==PORT_IMPL_ARM)
  GET_SYSTEM_TIME((timestamp+0),(timestamp+1));
#else
#error Macro 'PORT_IMPL' definition is needed
#endif
}

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)

/**
  * @brief Установка обработчиков
  */
static void vSetSignal( int iSignalNr, void (*pSigHandler)(int) )
{
  struct sigaction sa = {0};
  // Разрешаем сигнал SIGUSR1 и ставим обработчик
  sa.sa_handler = pSigHandler;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = 0;
  sigaction(iSignalNr, &sa, NULL);
}

/**
  * @brief Обработчик SIGTERM
  */
 static void handler_sigterm(int sig)
 {  
   switch ( sig ) {
     case SIGTERM: running = 0; break;
   }
 }
 
 /**
   * @brief Обработчик SIGINT
   */
 static void handler_sigint(int sig)
 {    
   switch ( sig ) {
     case SIGINT: running = 0; break;
   }
 }


#endif // PORT_IMPL_LINUX && LINUX_HW_IMPL_ARM
