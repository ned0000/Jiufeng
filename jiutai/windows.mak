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

SOURCES = jf_hex.c jf_process.c jf_time.c

!if "$(DEBUG_JIUFENG)" == "yes"
EXTRA_CFLAGS = -DDEBUG_RADIXTREE -DDEBUG_PRIOTREE -DDEBUG_WAITQUEUE \
               -DDEBUG_WORKQUEUE
!endif

EXTRA_INC_DIR = 

!include "$(TOPDIR)\mak\winobj.mak"

#-----------------------------------------------------------------------------


