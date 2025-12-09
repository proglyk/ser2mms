/**
  * @file   ser2mms_slave.c
  * @author Ilia Proniashin, msg@proglyk.ru
  * @date   09-October-2025
  */

#include "ser2mms.h"
#include "port_can.h"
#include "port_pps.h"
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>


static void write_carg_slave( void *, u16_t *, u32_t, u8_t, u8_t );
static void write_subs_slave( void *, prm_t *, u32_t );
static void read_answ_slave( void *, u16_t *, u32_t * );


volatile bool runned = true;
static s2m_t *s2m = NULL;

static rs485_init_t s2m_stty_init = {
#if (PORT_IMPL==PORT_IMPL_LINUX)
#if (LINUX_HW_IMPL==LINUX_HW_IMPL_WSL)
//.device_path = "/dev/ttyUSB0",
  .device_path = "/dev/ttyV1",
  .gpio_path   = NULL
#elif (LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  .device_path = "/dev/ttyS2",
  .gpio_path   = "/dev/gpiochip0",//"/dev/gpiochip3",
  .gpio_pin    = 13//26
#endif
#else
#error "Not available"
#endif
};


int main(void)
{
  char symb;
  
  // init
  s2m = ser2mms_new(NULL, write_carg_slave, write_subs_slave, read_answ_slave, 
                    S2M_SLAVE, 12, 
                    (void *)&s2m_stty_init);
  assert(s2m);
  
// #if defined(LINUX)&&defined(ARM)
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  pps_t pps = pps_new();
  if (!pps) printf("[main] pps=null\r\n");
  
  if (pps_run(pps) < 0) {
    printf("[main] can't run pps\r\n");
    pps_destroy(pps);
    exit(1);
  }
#endif

#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  can_t can_ptr = can_new("can1", 0);
  assert(can_ptr);
#endif
  
  // run
  ser2mms_run(s2m);
  printf("Runned\n");
  
  // loop
  do {
    printf( "> " );
    symb = getchar();
    switch (symb) {
      case 'q': runned = false; break;
    }
  } while( runned );
  
  // close
  //dio__deinit(gpio0_30);
#if (PORT_IMPL==PORT_IMPL_LINUX)&&(LINUX_HW_IMPL==LINUX_HW_IMPL_ARM)
  can_del(can_ptr);
  pps_stop(pps);
  pps_destroy(pps);
#endif
  ser2mms_stop(s2m);
  printf("Stopped\n");
  return 0; 
}

/**
  * @brief Функция обновления полей датасетов.
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  * @retval none: Нет
  */
static void write_carg_slave( void *opaque, u16_t *carg_buf, 
                              __UNUSED u32_t carg_len, u8_t ds, u8_t page )
{
  float ftemp;
  s32_t stemp;

#if (S2M_USE_LIBIEC==1)
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

  printf("[write_carg_slave] Value #1: %02d\r\n",  carg_buf[ 0]);
  printf("[write_carg_slave] Value #1: %02d\r\n",  carg_buf[ 1]);
  printf("[write_carg_slave] Value #1: %02d\r\n",  carg_buf[ 2]);

#endif // (S2M_USE_LIBIEC==1)
}

/**  ----------------------------------------------------------------------------
  * @brief Функция обновления полей подписок
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  * @retval none: Нет */
static void write_subs_slave( void *opaque, prm_t *subs_buf, 
                              __UNUSED u32_t subs_len )
{
  // f32_t ftemp;
  s2m_t *s2m = (s2m_t *)opaque;
  assert(s2m);
  
  printf("[write_subs_slave] Value #1: %02d\r\n",  subs_buf[ 0].sl);
  printf("[write_subs_slave] Epoch #1: %010d\r\n", subs_buf[ 0].pul[0]);
  printf("[write_subs_slave] Usec  #1: %d\r\n",  subs_buf[ 0].pul[1]);
  
  printf("[write_subs_slave] Epoch #2: %010d\r\n", subs_buf[ 1].pul[0]);
  printf("[write_subs_slave] Epoch #3: %010d\r\n", subs_buf[ 2].pul[0]);
  printf("[write_subs_slave] Epoch #4: %010d\r\n", subs_buf[ 3].pul[0]);
  printf("[write_subs_slave] Epoch #5: %010d\r\n", subs_buf[ 4].pul[0]);
  printf("[write_subs_slave] Epoch #6: %010d\r\n", subs_buf[ 5].pul[0]);
  printf("[write_subs_slave] Epoch #7: %010d\r\n", subs_buf[ 6].pul[0]);
  printf("[write_subs_slave] Epoch #8: %010d\r\n", subs_buf[ 7].pul[0]);
  printf("[write_subs_slave] Epoch #9: %010d\r\n", subs_buf[ 8].pul[0]);
  printf("[write_subs_slave] Epoch#10: %010d\r\n", subs_buf[ 9].pul[0]);
  printf("[write_subs_slave] Epoch#11: %010d\r\n", subs_buf[10].pul[0]);
  
  
/*   SET_ATTR(s32, p, IEDMODEL_UPG_GGIO0_ConnStatus_mag_i,     subs_buf[0].sl);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_ConnStatus_t, (u32_t*)subs_buf[0].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_ConnStatus_q, true);
  
  SET_ATTR(s32, p, IEDMODEL_UPG_GGIO0_HV_mag_i,     subs_buf[1].sl);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_HV_t, (u32_t*)subs_buf[1].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_HV_q, true);
  
  SET_ATTR(s32, p, IEDMODEL_UPG_GGIO0_LV_mag_i,     subs_buf[2].sl);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_LV_t, (u32_t*)subs_buf[2].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_LV_q, true);
  
  SET_ATTR(s32, p, IEDMODEL_UPG_GGIO0_Pause_mag_i,    subs_buf[3].sl);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_Pause_t, (u32_t*)subs_buf[3].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_Pause_q, true);
  
  SET_ATTR(s32, p, IEDMODEL_UPG_GGIO0_Protect1_mag_i,     subs_buf[4].sl);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_Protect1_t, (u32_t*)subs_buf[4].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_Protect1_q, true);
  
  // 5
  SET_ATTR(s32, p, IEDMODEL_UPG_GGIO0_Protect2_mag_i,     subs_buf[5].sl);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_Protect2_t, (u32_t*)subs_buf[5].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_Protect2_q, true);
  
  // 6
  SET_ATTR(s32, p, IEDMODEL_UPG_GGIO0_Ready_mag_i,     subs_buf[6].sl);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_Ready_t, (u32_t*)subs_buf[6].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_Ready_q, true);

  SET_ATTR(s32, p, IEDMODEL_UPG_GGIO0_Work_mag_i,     subs_buf[7].sl);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_Work_t, (u32_t*)subs_buf[7].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_Work_q, true);
  
  ftemp = (f32_t)subs_buf[8].sl
  SET_ATTR(f32, p, IEDMODEL_UPG_GGIO0_Id_mag_f,     ftemp);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_Id_t, (u32_t*)subs_buf[8].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_Id_q, true);

  ftemp = (f32_t)subs_buf[9].sl
  SET_ATTR(f32, p, IEDMODEL_UPG_GGIO0_Rl_mag_f,     ftemp);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_Rl_t, (u32_t*)subs_buf[9].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_Rl_q, true);
  
  ftemp = (f32_t)subs_buf[10].sl
  SET_ATTR(f32, p, IEDMODEL_UPG_GGIO0_Ud_mag_f,     ftemp);
  SET_ATTR(t,   p, IEDMODEL_UPG_GGIO0_Ud_t, (u32_t*)subs_buf[10].pul);
  SET_ATTR(q,   p, IEDMODEL_UPG_GGIO0_Ud_q, true);  */
}

/**  ----------------------------------------------------------------------------
  * @brief Функция обновления полей датасетов.
  * @param dataset: Номер текущего датасета посылки.
  * @param cargpage: Номер текущей страницы каретки.
  * @param ptr: Указатель на структуру со значениями из посылки.
  * @retval none: Нет */
static void read_answ_slave( void *argv, u16_t *answ_buf, u32_t *answ_len )
{
  // DataAttribute *data_attr;
  // f32_t ftemp;
  u32_t cnt = 0;
  
  (void)argv;
  (void)answ_buf;
  
/*   // Iz
  data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Iz_mag_f;
  ftemp = MmsValue_toFloat(data_attr->mmsValue);
  ftemp *= 1.0f;
  answ_buf[cnt++] = (u16_t)((s32_t)ftemp & 0x0000ffff);

  // Rlz
  data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Rlz_mag_f;
  ftemp = MmsValue_toFloat(data_attr->mmsValue);
  ftemp *= 1.0f;
  answ_buf[cnt++] = (u16_t)((s32_t)ftemp & 0x0000ffff);

  // usCirc
  data_attr = (DataAttribute*)IEDMODEL_UPG_GGIO0_Circ_mag_i;
  self->outdata.usCirc = (u16_t)(MmsValue_toInt32(data_attr->mmsValue) & 
    0x0000ffff);
  answ_buf[cnt++] = (u16_t)( MmsValue_toInt32(data_attr->mmsValue) & 0x0000ffff );
     */
  *answ_len = cnt;
}
