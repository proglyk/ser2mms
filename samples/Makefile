# Linked top library
LIBDIR    := ../..
LIB       := $(LIBDIR)/bin/ser2mms.a
LIBPERIPH := $(LIBDIR)/third/c-periphery/periphery.a
IECDIR   = ../../../mylibiec61850

# For sample (main) only
# CC       = gcc
CC       = arm-linux-gnueabihf-gcc
INCLUDES = -I$(LIBDIR)/include -I$(LIBDIR)/src/include \
           -I$(LIBDIR)/src/include/core -I$(LIBDIR)/src/include/port \
           -I$(IECDIR)/include
CFLAGS   = -Wall -g -Wextra $(INCLUDES)
LDFLAGS  = $(LIB) $(LIBPERIPH)
#LIBS     = ../../bin/mb.a ../../third/c-periphery/periphery.a
SLAVE     = ser2mms_slave
POLL      = ser2mms_poll

all: $(SLAVE)

slave: $(SLAVE)
poll: $(POLL)

$(SLAVE): $(LIB) $(SLAVE).c
	$(CC) $(CFLAGS) $(SLAVE).c $(LDFLAGS) -o $@

$(POLL): $(LIB) $(POLL).c
	$(CC) $(CFLAGS) $(POLL).c $(LDFLAGS) -o $@

$(LIB):
	$(MAKE) -C $(LIBDIR)

clean:
	rm -f $(SLAVE)
	rm -f $(POLL)
	$(MAKE) -C $(LIBDIR) clean
