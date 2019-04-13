#
#  @file windows.mak
#
#  @brief The makefile for jiukun library
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_jiukun

RESOURCE = jiukun

SOURCES = buddy.c slab.c mempool.c jiukun.c

JIUTAISRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\jf_mutex.c \
             $(JIUTAI_DIR)\waitqueue.c $(JIUTAI_DIR)\jf_sem.c

EXTRA_DEFS = -DJIUFENG_JIUKUN_DLL

EXTRA_LIBS = jf_logger.lib

!if "$(DEBUG_JIUFENG)" == "yes"
EXTRA_CFLAGS = -DDEBUG_JIUKUN
!endif

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



