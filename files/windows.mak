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

#---------------------------------------------------------------------------------------------------

DLLNAME = olfiles
RESOURCE = files

SOURCES = files.c directory.c conffile.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_time.c

EXTRA_LIBS = $(LIB_DIR)\jf_jiukun.lib

EXTRA_INC_DIR =

EXTRA_DEFS = -DJIUFENG_FILES_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



