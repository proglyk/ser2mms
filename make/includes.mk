
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
# IEC61850_HOME = $(SER2MMS_HOME)/../mylibiec61850
# LIB_INC_DIRS += $(IEC61850_HOME)/include
IEC61850_HOME = $(SER2MMS_HOME)/../../git/libiec61850
LIB_INC_DIRS += $(IEC61850_HOME)/src/iec61850/inc
LIB_INC_DIRS += $(IEC61850_HOME)/src/common/inc
LIB_INC_DIRS += $(IEC61850_HOME)/src/mms/inc
LIB_INC_DIRS += $(IEC61850_HOME)/src/logging
# LIB_INC_DIRS += $(IEC61850_HOME)/src/
# LIB_INC_DIRS += $(IEC61850_HOME)/src/
# LIB_INC_DIRS += $(IEC61850_HOME)/src/

LIB_INC_DIRS += $(IEC61850_HOME)/hal/inc
endif

CFLAGS += -DLIBIEC=$(LIBIEC)
