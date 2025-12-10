
SER2MMS_HOME = .

# Определение целевой платформы
include $(SER2MMS_HOME)/make/target.mk

# ========= Платформо-независимый код (PORT_IMPL определен в target.mk) ========

include $(SER2MMS_HOME)/make/includes.mk

LIB_SRC_DIRS  = src
LIB_SRC_DIRS += src/core
LIB_SRC_DIRS += src/core/transp_impl

# ========== Платформо-зависимый код (PORT_IMPL определен в target.mk) =========

ifeq ($(PORT_IMPL), LINUX)
ifeq ($(LINUX_HW_IMPL), WSL)
else ifeq ($(LINUX_HW_IMPL), ARM)
LIB_INC_DIRS += $(PERIPHERY_HOME)/src
endif
LIB_SRC_DIRS += src/port/linux
else ifeq ($(PORT_IMPL), WIN32)
LIB_SRC_DIRS += src/port/win32
else ifeq ($(PORT_IMPL), RTOS)
LIB_INC_DIRS += third/hal/header
LIB_INC_DIRS += third/project_inc
LIB_INC_DIRS += third/CMSIS
LIB_INC_DIRS += third/FreeRTOS/src/core
LIB_INC_DIRS += third/FreeRTOS/src/port
LIB_INC_DIRS += third/FreeRTOS/inc
LIB_SRC_DIRS += src/port/rtos
else ifeq ($(PORT_IMPL), BARE)
LIB_SRC_DIRS += src/port/bare
endif

# ========================== Генерация списка файлов ===========================

LIB_INCS = $(addprefix -I,$(LIB_INC_DIRS))
LIB_SRCS = $(foreach dir,$(LIB_SRC_DIRS),$(wildcard $(dir)/*.c))
LIB_OBJS = $(patsubst src/%,$(LIB_OBJS_DIR)/%, $(patsubst port/%, $(LIB_OBJS_DIR)/%, $(LIB_SRCS:.c=.o)))

$(shell mkdir -p $(LIB_BIN_DIR))
$(shell mkdir -p $(LIB_OBJS_DIR))

# ========================= Определение целей сборки ===========================

.PHONY: all lib samples clean

all: lib samples

lib: $(LIB_SER2MMS)

samples:
	$(MAKE) -C samples

# Правило связывания: .o > архив
$(LIB_SER2MMS): $(LIB_OBJS)
	$(AR) rcs $@ $^

# Правило компиляции .c > .o (с сохранением структуры каталогов)
$(LIB_OBJS_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(LIB_INCS) $< -o $@

$(LIB_OBJS_DIR)/%.o: port/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(LIB_INCS) $< -o $@

# ========================= Определение целей очистки ==========================

clean:
	rm -rf $(LIB_OBJS_DIR) $(LIB_BIN_DIR)
