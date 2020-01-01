#
#  @file utility/linux.mak
#
#  @brief The Makefile for utility.
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak

PROGRAMS = jf_errcode

SOURCES = errcode.c

include $(TOPDIR)/mak/lnxobjdef.mak

FULL_PROGRAMS = $(foreach i, $(PROGRAMS), $(BIN_DIR)/$i)

EXTRA_CFLAGS = -D_GNU_SOURCE

EXTRA_INC_DIR =

all: $(FULL_PROGRAMS)

$(BIN_DIR)/jf_errcode: errcode.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_string

include $(TOPDIR)/mak/lnxobjbld.mak

clean:
	rm -f $(FULL_PROGRAMS) $(OBJECTS)

#---------------------------------------------------------------------------------------------------


