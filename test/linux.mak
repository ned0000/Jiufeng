#
#  @file Makefile
#
#  @brief The Makefile for test
#
#  @author Min Zhang
#
#  @note
#

#-----------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak

PROGRAMS = xmalloc-test bases-test logger-test process-test xtime-test

SOURCES = xmalloc-test.c bases-test.c logger-test.c process-test.c \
          xtime-test.c

include $(TOPDIR)/mak/lnxobjdef.mak

FULL_PROGRAMS = $(foreach i, $(PROGRAMS), $(BIN_DIR)/$i)

EXTRA_CFLAGS = -D_GNU_SOURCE

EXTRA_INC_DIR =

all: $(FULL_PROGRAMS)

$(BIN_DIR)/xtime-test: xtime-test.o $(JIUTAI_DIR)/xtime.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolstringparse

$(BIN_DIR)/bases-test: bases-test.o $(JIUTAI_DIR)/bases.o \
       $(JIUTAI_DIR)/xmalloc.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lollogger

$(BIN_DIR)/logger-test: logger-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lollogger

$(BIN_DIR)/process-test: process-test.o $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lollogger

$(BIN_DIR)/xmalloc-test: xmalloc-test.o $(JIUTAI_DIR)/xmalloc.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS)

include $(TOPDIR)/mak/lnxobjbld.mak

clean:
	rm -f $(FULL_PROGRAMS) $(OBJECTS)

#-----------------------------------------------------------------------------


