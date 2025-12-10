/**
 * @file ser.h
 * @author Ilia Proniashin, msg@proglyk.ru
 * @date 30-September-2025
 */

#ifndef SER2MMS_SER_H
#define SER2MMS_SER_H

#include "ser2mms_conf.h"
#include "types.h"
#include "port_types.h"

// Отладочный вывод
#define SER_DEBUG S2M_DEBUG

//#define MB_SER_PDU_SIZE_MAX (16) // Maximum size of a Modbus RTU frame.
#define REGS_NUM (125)

// Размер таблицы (число) доступных для чтения и записи (R/W) регистров.
#define MB_INPUT_SIZE REGS_NUM
#define MB_HOLD_SIZE REGS_NUM

// Размер буферов
#define BUFSIZE (2*REGS_NUM)

// Short names
#define MB_RCV_BUF(S) (GET_RCVD(S)->buf)
#define MB_RCV_POS(S) (GET_RCVD(S)->pos)
#define MB_RCV_SIZE(S) (GET_RCVD(S)->size)
#define MB_XMT_BUF(S) (GET_XMIT(S)->buf)
#define MB_XMT_POS(S) (GET_XMIT(S)->pos)
#define MB_XMT_SIZE(S) (GET_XMIT(S)->size)

// Short names for func declarations
//#define buf_t ser_buf_t // TODO слишком общно

// Short names for func declarations
#define GET_RCVD(S) ser_get_buf_rcvd(S)
#define GET_XMIT(S) ser_get_buf_xmit(S)

// Struct declaration
struct ser_buf_s {
  u8_t *p;
  u32_t pos;
  u32_t size;
};

struct buf_rcvd_s {
  u8_t buf[BUFSIZE];
  u32_t pos;
  u32_t size;
};

struct buf_xmit_s {
  u8_t buf[BUFSIZE+10];
  u32_t pos;
  u32_t size;
};

// Type declaration
typedef struct buf_rcvd_s *buf_rcvd_t;
typedef struct buf_xmit_s *buf_xmit_t;
typedef struct ser_buf_s *ser_buf_t;
typedef struct ser_s *ser_t;

/**
 * @brief Создает новый экземпляр объекта 'struct ser_s'
 * @param mode Режим работы (MODE_POLL или MODE_SLAVE)
 * @param carg_cb Callback-функция обновления параметров/данных
 * @param subs_cb Callback-функция обновления параметров подписок
 * @param answ_cb Callback-функция генерации ответа
 * @param pld_api Указатель на контекст API полезной нагрузки
 * @retval Возвращает указатель на объект 'struct ser_s' и NULL если не удалось 
 * выделить память или объект уже используется (статичная аллокация)
 */
ser_t ser_new(topmode_t mode, carg_fn_t carg_cb, subs_fn_t subs_cb, 
              answ_fn_t answ_cb, void *pld_api);

/**
 * @brief Уничтожает объект 'struct ser_s' и освобождает ресурсы
 * @param self Указатель на экземпляр обработчика
 */
void ser_del(ser_t self);

/**
 * @brief Устанавливает тип команды для следующей передачи
 * @param self Указатель на экземпляр обработчика
 * @param value Значение команды (0 - CMD_PARAMETERS, ненулевое - CMD_TIMESET)
 */
void ser_set_cmd(ser_t self, u32_t value);

/**
 * @brief Разбирает принятое входящее сообщение и обрабатывает полезную нагрузку
 * @param self Указатель на экземпляр обработчика
 * @retval 0 при успехе
 * @retval -1 при ошибке (неверный размер, датасет или индекс страницы)
 */
s32_t ser_in_parse(ser_t self);

/**
 * @brief Формирует исходящее сообщение с заголовком и полезной нагрузкой для 
 * передачи
 * @param self Указатель на экземпляр обработчика
 */
void ser_out_build(ser_t self);

/**
 * @brief Возвращает указатель на структуру буфера приема
 * @param self Указатель на экземпляр обработчика
 * @retval buf_rcvd_t Указатель на буфер приема
 */
buf_rcvd_t ser_get_buf_rcvd(ser_t self);

/**
 * @brief Возвращает указатель на структуру буфера передачи
 * @param self Указатель на экземпляр обработчика
 * @retval buf_xmit_t Указатель на буфер передачи
 */
buf_xmit_t ser_get_buf_xmit(ser_t self);

#endif
