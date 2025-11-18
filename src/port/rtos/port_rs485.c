/**
 * @file port_rs485.c
 * @author Ilia Proniashin, mail@proglyk.ru
 * @date 16-November-2025
 */

// #ifndef USE_FREERTOS
// #error "Should only be compiled with FreeRTOS enabled"
// #endif

#include "port_rs485.h"
#include "port_rs485_init.h"
#include "port_alloc.h"
//#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Макрос RS485_USE_STATIC должен быть выкл. для динамической аллокации
#if (RS485_USE_STATIC)
#error "Macro 'RS485_USE_STATIC' must be disabled"
#endif

#ifndef RCVD_BUF_SIZE
#define RCVD_BUF_SIZE 256
#endif

#ifndef XMIT_BUF_SIZE
#define XMIT_BUF_SIZE 256
#endif

typedef enum { DIR_IN = 0, DIR_OUT = 1 } dir_t;

// Структура управления GPIO для nRE/DE
typedef struct {
  GPIO_TypeDef *port;
  u32_t pin;
  dir_t mode;
} dio_t;

struct rs485_s {
  UART_HandleTypeDef huart;
  bool sta_ena_rx;
  u8_t rcvd_buf[RCVD_BUF_SIZE];
  u32_t rcvd_pos;
  fn_t fn_rcv;
  void *fn_pld;
  bool sta_ena_tx;
  u8_t xmit_buf[XMIT_BUF_SIZE];
  u32_t xmit_size;
  fn_t fn_xmt;
  // dio_t *nre_de;
  dio_t nre_de;
};
typedef struct rs485_s rs485_t;

static bool  receive(UART_HandleTypeDef *, u8_t *, u32_t, u32_t *);
static bool  transmit(UART_HandleTypeDef *, const u8_t *, u32_t);
static s32_t nre_de_init(rs485_inst_t, GPIO_TypeDef *, u32_t);
static s32_t nre_de_set(rs485_inst_t, dir_t);
static void  nre_de_del(rs485_inst_t);

PORT_STATIC_DECLARE(RS485, rs485_t);

//================================ PUBLIC API ==================================

/**
  * @brief Constructor of 'rs485_inst_t' object
  * @param uart_instance - UART instance (USART1, USART2, etc.)
  * @param baudrate - UART baudrate (e.g., 9600, 115200)
  * @param fn_rcv - Receive callback function
  * @param fn_xmt - Transmit callback function
  * @param fn_pld - Callback payload pointer
  * @param gpio_port - GPIO port for nRE/DE pin (e.g., GPIOA)
  * @param gpio_pin - GPIO pin number (0-15)
  * @return Pointer to rs485_inst_t object or NULL on error
  */
rs485_inst_t rs485_new(void *init, rs485_fn_t *fn)
{
  rs485_init_t *pinit = (rs485_init_t *)init;
  assert(pinit && fn);
  
  PORT_ALLOC(RS485, rs485_t, self, return NULL);
  
  self->fn_rcv = fn->func_rcv;
  self->fn_xmt = fn->func_xmt;
  self->fn_pld = fn->pld;
  
  // Настройка структуры UART
  self->huart.Instance = pinit->uart;
  self->huart.Init.BaudRate = pinit->baudrate;
  self->huart.Init.WordLength = UART_WORDLENGTH_8B;
  self->huart.Init.StopBits = UART_STOPBITS_2;
  self->huart.Init.Parity = UART_PARITY_NONE;
  self->huart.Init.Mode = UART_MODE_TX_RX;
  self->huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  self->huart.Init.OverSampling = UART_OVERSAMPLING_16;
  
  // Инициализация UART через HAL
  if (HAL_UART_Init(&self->huart) != HAL_OK) {
    printf("[rs485_new] HAL_UART_Init failed\r\n");
    goto exit_0;
  }
  
  // Инициализация управления nRE/DE
  if (pinit->gpio_port) {
    if (nre_de_init(self, pinit->gpio_port, pinit->gpio_pin) < 0) {
      printf("[rs485_new] Can't init nRE/DE control\r\n");
      goto exit_1;
    }
  }
  
  printf("[rs485_new]\n");
  return self;

exit_1:
  HAL_UART_DeInit(&self->huart);
exit_0:
  PORT_FREE(RS485, self);
  printf("[rs485_new] err\n");
  return NULL;
}

/**
  * @brief Destructor
  * @param self - Pointer to rs485_inst_t object
  */
void rs485_del(rs485_inst_t self)
{
  assert(self);
  nre_de_del(self);
  HAL_UART_DeInit(&self->huart);
  printf("[rs485_del]\n");
  PORT_FREE(RS485, self);
}

/**
  * @brief Enable/disable RX and TX
  * @param self - Pointer to rs485_inst_t object
  * @param ena_rx - Enable receiver
  * @param ena_tx - Enable transmitter
  */
void rs485_ena(rs485_inst_t self, bool ena_rx, bool ena_tx)
{
  assert(ena_rx ^ ena_tx);
  
  self->sta_ena_rx = ena_rx;
  if (ena_rx) {
    // Очистка буфера приема
    __HAL_UART_FLUSH_DRREGISTER(&self->huart);
    self->rcvd_pos = 0;
  }
  
  self->sta_ena_tx = ena_tx;
  if (ena_tx) {
    self->xmit_size = 0;
  }
}

/**
  * @brief Get byte from receive buffer
  * @param self - Pointer to rs485_inst_t object
  * @param byte - Pointer to store received byte
  * @return true if byte retrieved, false if buffer empty
  */
bool rs485_get(rs485_inst_t self, u8_t *byte)
{
  if (self->rcvd_pos < RCVD_BUF_SIZE) {
    *byte = self->rcvd_buf[self->rcvd_pos++];
    return true;
  }
  return false;
}

/**
 * @brief Put byte into transmit buffer
 * @param self - Pointer to rs485_inst_t object
 * @param byte - Byte to transmit
 * @return true if byte added, false if buffer full
 */
bool rs485_put(rs485_inst_t self, u8_t byte)
{
  assert(self);
  if (self->xmit_size < XMIT_BUF_SIZE) {
    self->xmit_buf[self->xmit_size++] = byte;
    return true;
  }
  return false;
}

/**
  * @brief Poll serial port for RX/TX operations
  * @param self - Pointer to rs485_inst_t object
  */
void rs485_poll(rs485_inst_t self)
{
  u32_t rcvd = 0;
  u32_t i;
  
  if (self->sta_ena_rx) {
    if (!receive(&self->huart, self->rcvd_buf, RCVD_BUF_SIZE, &rcvd)) {
      return;
    }
    
    if (rcvd > 0) {
      for (i = 0; i < rcvd; i++) {
        /* Call the modbus stack and let him fill the buffers. */
        if (self->fn_rcv) self->fn_rcv(self->fn_pld);
      }
      self->rcvd_pos = 0;
    }
  }
  
  if (self->sta_ena_tx) {
    while (self->sta_ena_tx) {
      if (self->fn_xmt) self->fn_xmt(self->fn_pld);
    }
    
    nre_de_set(self, DIR_OUT);
      
    if (!transmit(&self->huart, self->xmit_buf, self->xmit_size)) {
      printf("Can't send the frame completely\r\n");
    }
    
    nre_de_set(self, DIR_IN);
  }
}

//=============================== PRIVATE API ==================================

/**
  * @brief Receive data from UART
  * @param huart - HAL UART handle
  * @param buf - Buffer to store received data
  * @param size - Buffer size
  * @param rcvd - Pointer to store number of bytes received
  * @return true on success, false on error
  */
static bool receive(UART_HandleTypeDef *huart, u8_t *buf, u32_t size, u32_t *rcvd)
{
  u32_t count = 0;
  
  // Неблокирующее чтение доступных байт
  while (count < size && __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE)) {
    buf[count] = (u8_t)(huart->Instance->DR & 0xFF);
    count++;
  }
  
  *rcvd = count;
  return true;
}

/**
  * @brief Transmit data to UART
  * @param huart - HAL UART handle
  * @param buf - Buffer with data to transmit
  * @param size - Number of bytes to transmit
  * @return true on success, false on error
  */
static bool transmit(UART_HandleTypeDef *huart, const u8_t *buf, u32_t size)
{
  HAL_StatusTypeDef status;
  
  // Блокирующая передача с таймаутом
  status = HAL_UART_Transmit(huart, (u8_t *)buf, size, 1000);
  
  if (status != HAL_OK) {
    return false;
  }
  
  // Ожидание завершения передачи
  while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET) {
    vTaskDelay(1);
  }
  
  return true;
}

/**
  * @brief Pin nRE/DE initializer
  * @param self - Pointer to rs485_inst_t object
  * @param gpio_port - GPIO port (e.g., GPIOA)
  * @param gpio_pin - GPIO pin number (0-15)
  * @return 0 on success, -1 on error
  */
static s32_t nre_de_init(rs485_inst_t self, GPIO_TypeDef *gpio_port, u32_t gpio_pin)
{
  assert(self);
  
  // // Выделяем память для структуры dio
  // self->nre_de = calloc(1, sizeof(dio_t));
  // if (!self->nre_de) {
    // printf("[nre_de_init] Can't allocate memory\r\n");
    // return -1;
  // }
  
  self->nre_de.port = gpio_port;
  self->nre_de.pin = gpio_pin;
  
  // Включаем тактирование GPIO
  if (gpio_port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
  else if (gpio_port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
  else if (gpio_port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
  else if (gpio_port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
  else if (gpio_port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
  else if (gpio_port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
  else if (gpio_port == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
  else if (gpio_port == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
  
  // Настраиваем GPIO
  GPIO_InitTypeDef gpio_init;
  gpio_init.Pin = (1 << gpio_pin);
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  
  HAL_GPIO_Init(gpio_port, &gpio_init);
  
  // Устанавливаем в режим приема (low)
  nre_de_set(self, DIR_IN);
  
  return 0;
}

/**
  * @brief Pin nRE/DE deinitializer
  * @param self - Pointer to rs485_inst_t object
  */
static void nre_de_del(rs485_inst_t self)
{
  assert(self);
  HAL_GPIO_DeInit(self->nre_de.port, (1 << self->nre_de.pin));
}

/**
  * @brief Set the nRE/DE state (IN or OUT)
  * @param self - Pointer to rs485_inst_t object
  * @param direction - Direction (DIR_IN for receive, DIR_OUT for transmit)
  * @return 0 on success, -1 on error
  */
static s32_t nre_de_set(rs485_inst_t self, dir_t direction)
{
  assert(self);
  
  self->nre_de.mode = direction;
  
  // Вычисляем маску для пина (2^pin)
  u32_t pin_mask = (1 << self->nre_de.pin);
  
  switch (direction) {
    case DIR_IN:  // Прием - устанавливаем LOW (nRE=0, DE=0)
      self->nre_de.port->ODR &= ~pin_mask;
      break;
      
    case DIR_OUT: // Передача - устанавливаем HIGH (nRE=1, DE=1)
      self->nre_de.port->ODR |= pin_mask;
      break;
      
    default:
      return -1;
  }
  
  return 0;
}
