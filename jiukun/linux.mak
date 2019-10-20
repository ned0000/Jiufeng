#
#  @file linux.mak
#
#  @brief The makefile for jiukun library
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_jiukun

SOURCES = buddy.c slab.c jiukun.c

JIUTAISRCS = jf_mem.c jf_mutex.c

EXTRA_LIBS = -ljf_logger

ifeq ("$(DEBUG_JIUFENG)", "yes")
EXTRA_CFLAGS += -DDEBUG_JIUKUN
EXTRA_CFLAGS += -DDEBUG_JIUKUN_STAT
EXTRA_CFLAGS += -DDEBUG_JIUKUN_VERBOSE
endif

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

