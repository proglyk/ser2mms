
# ===== Определение ARCH и OS по умолчанию (если аргументы не заданы явно) =====

ifndef ARCH
ARCH = x86_64
endif

ifndef OS
OS = linux
endif

# ==================== Определение PORT_IMPL на основе OS ======================

PORT_IMPL =

ifeq ($(OS), linux)
PORT_IMPL = LINUX
endif

ifeq ($(OS), bare)
PORT_IMPL = BARE
endif

ifeq ($(OS), rtos)
PORT_IMPL = RTOS
endif

# ================== Настройка тулчейна на основе ARCH + OS ====================

LINUX_HW_IMPL =

# Архитектурные префиксы
WSL_LINUX_TOOLCHAIN =
ARM_LINUX_TOOLCHAIN = arm-linux-gnueabihf-
ARM_BARE_TOOLCHAIN = arm-none-eabi-

# x86_64 + Linux
ifeq ($(ARCH)-$(OS), x86_64-linux)
LINUX_HW_IMPL = WSL
TOOLCHAIN_PREFIX = $(WSL_LINUX_TOOLCHAIN)
CFLAGS += -m64 -pthread
CFLAGS += -Wall -g -Wextra
CFLAGS += -fsanitize=address -fstack-protector-strong
endif

# ARM + Linux
ifeq ($(ARCH)-$(OS), arm-linux)
LINUX_HW_IMPL = ARM
TOOLCHAIN_PREFIX = $(ARM_LINUX_TOOLCHAIN)
CFLAGS += -mcpu=cortex-a8 -mfloat-abi=hard -mfpu=neon
CFLAGS += -Wall -g -Wextra
endif

# ARM + bare-metal
# ifeq ($(ARCH)-$(OS), arm-bare)
# TOOLCHAIN_PREFIX = $(ARM_BARE_TOOLCHAIN)
# CFLAGS += -mcpu=cortex-m4 -mthumb
# endif

# ARM + FreeRTOS
# ifeq ($(ARCH)-$(OS), arm-rtos)
# TOOLCHAIN_PREFIX = $(ARM_BARE_TOOLCHAIN)
# # CFLAGS += -mcpu=cortex-m4 -mthumb -DFREERTOS
# CFLAGS += -Wall -g -Wextra -mcpu=cortex-m4 -mthumb -DARM_FREERTOS
# CFLAGS += -DSTM32F407xx -DARM_RTOS
# endif

# PORT_IMPL объявлен всегда и имеет значение, отражающее выбранную плафторму
CFLAGS += -DPORT_IMPL=$(PORT_IMPL)
# LINUX_HW_IMPL объявлен всегда, но имеет значение только, если PORT_IMPL==LINUX
CFLAGS += -DLINUX_HW_IMPL=$(LINUX_HW_IMPL)

# ====================== Компилятор и директории сборки ========================

LIB_BIN_DIR = $(SER2MMS_HOME)/bin
LIB_OBJS_DIR = $(SER2MMS_HOME)/build
# Определяем путь к библиотеке (важно для примеров!)
LIB_SER2MMS = $(LIB_BIN_DIR)/ser2mms.a

CC = $(TOOLCHAIN_PREFIX)gcc
AR = $(TOOLCHAIN_PREFIX)ar
