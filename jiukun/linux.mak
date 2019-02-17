#
#  @file linux.mak
#
#  @brief The makefile for jiukun library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SONAME = oljiukun

SOURCES = buddy.c slab.c jiukun.c

JIUTAISRCS = xmalloc.c syncmutex.c

EXTRA_LIBS = -lollogger

ifeq ("$(DEBUG_JIUFENG)", "yes")
EXTRA_CFLAGS += -DDEBUG_JIUKUN
EXTRA_CFLAGS += -DDEBUG_JIUKUN_STAT
#EXTRA_CFLAGS += -DDEBUG_JIUKUN_VERBOSE
endif

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------
