
# ========== Заголовочные файлы платформо-независимого ядра библиотеки =========

LIB_INC_DIRS  = $(SER2MMS_HOME)/include
LIB_INC_DIRS += $(SER2MMS_HOME)/src/core/include
LIB_INC_DIRS += $(SER2MMS_HOME)/src/port/include

# ===================== Подключение сторонних библиотек, =======================
# ================ шаренных между самой библиотекой и примерами ================

PERIPHERY_HOME = $(SER2MMS_HOME)/third/c-periphery

ifndef LIBIEC
LIBIEC = 1
endif

ifeq ($(LIBIEC), 1)
IEC61850_HOME = $(SER2MMS_HOME)/../mylibiec61850_3
LIB_INC_DIRS += $(IEC61850_HOME)/include
endif

CFLAGS += -DLIBIEC=$(LIBIEC)
