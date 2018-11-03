#
#  @file Makefile
#
#  @brief The Makefile for Jiutai
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SOURCES = hexstr.c process.c xtime.c bases.c xmalloc.c syncmutex.c \
    syncrwlock.c syncsem.c array.c hash.c

EXTRA_CFLAGS = -D_GNU_SOURCE

ifeq ("$(DEBUG_JIUFENG)", "yes")
EXTRA_CFLAGS += -DDEBUG_RADIXTREE -DDEBUG_PRIOTREE -DDEBUG_WAITQUEUE \
    -DDEBUG_WORKQUEUE -DDEBUG_AVLTREE
endif

EXTRA_INC_DIR = 

include $(TOPDIR)/mak/lnxobj.mak

#-----------------------------------------------------------------------------


