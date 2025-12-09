SER2MMS_HOME  = .
include make/target.mk

LIB_BIN_DIR   = bin
LIB_OBJS_DIR  = build
LIB_NAME      = $(LIB_BIN_DIR)/ser2mms.a

# Платформо-независимое ядро
LIB_INC_DIRS  = include
LIB_INC_DIRS += src/core/include
LIB_INC_DIRS += src/port/include
LIB_INC_DIRS += third/libiec61850/include
LIB_SRC_DIRS  = src
LIB_SRC_DIRS += src/core
LIB_SRC_DIRS += src/core/transp_impl

# Платформо-зависимый HAL-слой (HAL_IMPL определен в target.mk)
ifeq ($(HAL_IMPL), POSIX)
LIB_SRC_DIRS += src/port/linux
LIB_INC_DIRS += ./third/c-periphery/src
else ifeq ($(HAL_IMPL), WIN32)
LIB_SRC_DIRS += src/port/win32
else ifeq ($(HAL_IMPL), RTOS)
LIB_INC_DIRS += third/hal/header
LIB_INC_DIRS += third/project_inc
LIB_INC_DIRS += third/CMSIS
LIB_INC_DIRS += third/FreeRTOS/src/core
LIB_INC_DIRS += third/FreeRTOS/src/port
LIB_INC_DIRS += third/FreeRTOS/inc
LIB_SRC_DIRS += src/port/rtos
else ifeq ($(HAL_IMPL), BARE)
LIB_SRC_DIRS += src/port/bare
endif

LIB_INCS = $(addprefix -I,$(LIB_INC_DIRS))
LIB_SRCS = $(foreach dir,$(LIB_SRC_DIRS),$(wildcard $(dir)/*.c))
LIB_OBJS = $(patsubst src/%,$(LIB_OBJS_DIR)/%, $(patsubst port/%, $(LIB_OBJS_DIR)/%, $(LIB_SRCS:.c=.o)))

$(shell mkdir -p $(LIB_BIN_DIR))
$(shell mkdir -p $(LIB_OBJS_DIR))

all: $(LIB_NAME)

# Правило связывания: .o > исполняемый файл
$(LIB_NAME): $(LIB_OBJS)
	$(AR) rcs $@ $^

# Правило компиляции .c > .o (с сохранением структуры каталогов)
$(LIB_OBJS_DIR)/%.o: src/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(LIB_INCS) $< -o $@

$(LIB_OBJS_DIR)/%.o: port/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(LIB_INCS) $< -o $@

clean:
	rm -rf $(LIB_OBJS_DIR) $(LIB_BIN_DIR)

debug:
	@echo "=== Makefile Debug Info ==="
	@echo "ARCH:        $(ARCH)"
	@echo "OS:          $(OS)"
	@echo "HAL_IMPL:    $(HAL_IMPL)"
	@echo "CC:          $(CC)"
	@echo "CFLAGS:      $(CFLAGS)"
	@echo ""
	@echo "=== Source Directories ==="
	@$(foreach dir,$(LIB_SRC_DIRS),echo "  $(dir)";)
	@echo ""
	@echo "=== Source Files ($(words $(LIB_SRCS)) files) ==="
	@$(foreach src,$(LIB_SRCS),echo "  $(src)";)
	@echo ""
	@echo "=== Object Files ($(words $(LIB_OBJS)) files) ==="
	@$(foreach obj,$(LIB_OBJS),echo "  $(obj)";)