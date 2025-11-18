# ==============================================================================
# Двумерная система: ARCH + OS
# ==============================================================================
UNAME := $(shell uname)

# Архитектурные префиксы
ARM_LINUX_TOOLCHAIN = arm-linux-gnueabihf-
ARM_BARE_TOOLCHAIN = arm-none-eabi-
X86_MINGW_TOOLCHAIN = x86_64-w64-mingw32-

# ==============================================================================
# Определение ARCH и OS (если не заданы явно)
# ==============================================================================

ifndef ARCH
ARCH = x86_64  # По умолчанию
endif

ifndef OS
ifeq ($(UNAME), Linux)
OS = linux
else ifeq ($(findstring MINGW,$(UNAME)), MINGW)
OS = win32
else
OS = linux
endif
endif

# ==============================================================================
# Определение HAL_IMPL на основе OS (не ARCH!)
# ==============================================================================

ifeq ($(OS), linux)
HAL_IMPL = POSIX
endif

ifeq ($(OS), win32)
HAL_IMPL = WIN32
endif

ifeq ($(OS), bare)
HAL_IMPL = BARE
endif

ifeq ($(OS), rtos)
HAL_IMPL = RTOS
endif

# ==============================================================================
# Настройка тулчейна на основе ARCH + OS
# ==============================================================================

# x86_64 + Linux
ifeq ($(ARCH)-$(OS), x86_64-linux)
TOOLCHAIN_PREFIX =
CFLAGS += -m64
endif

# x86_64 + Windows (кросс-компиляция)
ifeq ($(ARCH)-$(OS), x86_64-win32)
TOOLCHAIN_PREFIX = $(X86_MINGW_TOOLCHAIN)
CFLAGS += -DWIN32
endif

# ARM + Linux
ifeq ($(ARCH)-$(OS), arm-linux)
TOOLCHAIN_PREFIX = $(ARM_LINUX_TOOLCHAIN)
CFLAGS += -march=armv7-a
CFLAGS += -DARM_LINUX
endif

# ARM + bare-metal
ifeq ($(ARCH)-$(OS), arm-bare)
TOOLCHAIN_PREFIX = $(ARM_BARE_TOOLCHAIN)
CFLAGS += -mcpu=cortex-m4 -mthumb
endif

# ARM + FreeRTOS
ifeq ($(ARCH)-$(OS), arm-rtos)
TOOLCHAIN_PREFIX = $(ARM_BARE_TOOLCHAIN)
# CFLAGS += -mcpu=cortex-m4 -mthumb -DFREERTOS
CFLAGS += -Wall -g -Wextra -mcpu=cortex-m4 -mthumb -DARM_FREERTOS
CFLAGS += -DSTM32F407xx -DARM_RTOS
endif

# ==============================================================================
# Опционально: MCU-specific настройки через переменную MCU
# ==============================================================================

ifdef MCU
ifeq ($(MCU), STM32F407)
CFLAGS += -DSTM32F407xx -DSTM32F4
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
endif

ifeq ($(MCU), STM32F103)
CFLAGS += -DSTM32F103xB -DSTM32F1
CFLAGS += -mfloat-abi=soft
endif
endif

# ==============================================================================
# Компилятор и директории сборки
# ==============================================================================

CC = $(TOOLCHAIN_PREFIX)gcc
AR = $(TOOLCHAIN_PREFIX)ar

# Отдельная директория для каждой комбинации ARCH-OS
LIB_OBJS_DIR = $(SER2MMS_HOME)/build_$(ARCH)_$(OS)

ifdef MCU
LIB_OBJS_DIR = $(SER2MMS_HOME)/build_$(ARCH)_$(OS)_$(MCU)
endif
