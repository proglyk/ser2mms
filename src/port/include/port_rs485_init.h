/**
  * @file   port_rs485_init.h
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date   14-November-2025
  */

#ifndef PORT_RS485_INIT_H
#define PORT_RS485_INIT_H

#include "port_types.h"

#ifdef ARM_RTOS
#include "stm32f4xx_hal.h"
#elif defined(ARM_LINUX)
//
#endif

typedef struct {
#ifdef ARM_RTOS
  USART_TypeDef *uart;
  u32_t baudrate;
  GPIO_TypeDef *gpio_port;
  u32_t gpio_pin;
#elif defined(ARM_LINUX)
  const char *device_path;      // "/dev/ttyS4"
  const char *gpio_path;        // "/sys/class/gpio/gpio60"
  u32_t gpio_pin;
#endif
  fn_t tx_callback;
  fn_t rx_callback;
  void *user_data;
} rs485_init_t;

#endif