#
#  @file jiukun/windows.mak
#
#  @brief The makefile for jiukun library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_jiukun

RESOURCE = jiukun

SOURCES = buddy.c slab.c jiukun.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\jf_mutex.c

EXTRA_DEFS = /DJIUFENG_JIUKUN_DLL

EXTRA_LIBS = jf_logger.lib

!if "$(DEBUG_JIUFENG)" == "yes"
#EXTRA_CFLAGS = $(EXTRA_CFLAGS) /DDEBUG_JIUKUN
#EXTRA_CFLAGS = $(EXTRA_CFLAGS) /DDEBUG_JIUKUN_STAT
#EXTRA_CFLAGS = $(EXTRA_CFLAGS) /DDEBUG_JIUKUN_VERBOSE
!endif

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



