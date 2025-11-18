/**
  * @file   ser2mms_conf.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   22-September-2025
  */
  
#ifndef SER2MMS_CONF_H
#define SER2MMS_CONF_H

// Enable threads
#define S2M_USE_THREADS                 (1)

// Don't use dynamically allocated objects
#define S2M_USE_STATIC                  (0)

// Var
#define S2M_USE_TRANSP_RTU              (1)
#define S2M_USE_TRANSP_TCP              (0)

// Use real libiec or imitate this one
#define LIBIEC_EXIST                    (1)

// Set the type of CRC calculation method
#define CRC_YURA                        (0) // Use the reverse type
#define CRC_MODBUS                      (1) // Use the classic type

#define SER_MIN_DS_IDX                  (1)
#define SER_MAX_DS_IDX                  (6)
#define SER_MIN_PAGE_IDX                (0)
#define SER_MAX_PAGE_IDX                (3)
#define SER_PAGE_SIZE                   (3)
#define SER_NUM_SUBS                    (11)
#define IN_MSG_SIZE_SLAVE               (100) //12
#define IN_MSG_SIZE_POLL                (8)
#define SER_ANSW_SIZE                   (3)

#endif //SER2MMS_CONF_H
