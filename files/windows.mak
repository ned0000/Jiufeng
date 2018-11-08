#
#  @file windows.mak
#
#  @brief the makefile for files library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = olfiles
RESOURCE = files

SOURCES = files.c directory.c

JIUTAISRCS = $(JIUTAI_DIR)\xmalloc.c $(JIUTAI_DIR)\xtime.c

EXTRA_INC_DIR =

EXTRA_DEFS = -DJIUFENG_FILES_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



