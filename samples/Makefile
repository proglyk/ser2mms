
SER2MMS_HOME = ..

include $(SER2MMS_HOME)/make/target.mk
include $(SER2MMS_HOME)/make/includes.mk

# ====================== Подключение сторонних библиотек =======================

# Constantly connected c-periphery
LIB_PERIPHERY = $(PERIPHERY_HOME)/periphery.a
ifeq ($(PORT_IMPL), LINUX)
ifeq ($(LINUX_HW_IMPL), ARM)
LIB_INC_DIRS += $(PERIPHERY_HOME)/src
endif
endif
LDFLAGS = $(LIB_SER2MMS) $(LIB_PERIPHERY)

# Conditionally connected libiec61850
ifeq ($(LIBIEC), 1)
LIB_IEC61850 = $(IEC61850_HOME)/build-arm/libiec61850.a
# LIBASN1  := $(IEC61850_HOME)/third/ASN1/bin/asn1.a
# LIB8823  := $(IEC61850_HOME)/third/ISO8823/bin/iso8823.a
# LIB8650  := $(IEC61850_HOME)/third/ISO8650/bin/iso8650.a
# LIB9506  := $(IEC61850_HOME)/third/ISO9506/bin/iso9506.a
# Additional includes
# LIB_INC_DIRS += $(IEC61850_HOME)/third/ASN1/include/ASN1
# LIB_INC_DIRS += $(IEC61850_HOME)/third/ISO8823/include/iso8823
# LIB_INC_DIRS += $(IEC61850_HOME)/third/ISO8650/include/iso8650
# LIB_INC_DIRS += $(IEC61850_HOME)/third/ISO9506/include/iso9506
# Additional link flags
# LDFLAGS += $(LIB_IEC61850) $(LIB8823) $(LIB8650) $(LIB9506) $(LIBASN1)
LDFLAGS += $(LIB_IEC61850)
endif

# совместимость
LDFLAGS += -static

# Sample binaries
SLAVE = ser2mms_slave
POLL = ser2mms_poll

INCLUDES = $(addprefix -I,$(LIB_INC_DIRS))

# ========================= Определение целей сборки ===========================

.PHONY: all slave poll clean clean_all

all: $(SLAVE) $(POLL)

slave: $(SLAVE)

poll: $(POLL)

$(SLAVE): $(LIB_SER2MMS) $(LIB_PERIPHERY) $(SLAVE).c
	$(CC) $(CFLAGS) $(SLAVE).c $(INCLUDES) $(LDFLAGS) -o $@

$(POLL): $(LIB_SER2MMS) $(LIB_PERIPHERY) $(POLL).c
	$(CC) $(CFLAGS) $(POLL).c $(INCLUDES) $(LDFLAGS) -o $@

$(LIB_SER2MMS):
	$(MAKE) -C $(SER2MMS_HOME)

$(LIB_PERIPHERY):
	$(MAKE) -C $(PERIPHERY_HOME) CROSS_COMPILE=$(TOOLCHAIN_PREFIX)

# ========================= Определение целей очистки ==========================

clean:
	rm -f $(SLAVE)
	rm -f $(POLL)

clean_all:
	rm -f $(SLAVE)
	rm -f $(POLL)
	$(MAKE) -C $(SER2MMS_HOME) clean
	$(MAKE) -C $(PERIPHERY_HOME) clean
