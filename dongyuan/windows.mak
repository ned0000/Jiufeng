#
#  @file windows.mak
#
#  @brief The main makefile for dongyuan
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

EXE = oldongyuan.exe

SOURCES = dongyuan.c main.c

JIUTAI_SRCS = $(JIUTAI_DIR)\getopt.c $(JIUTAI_DIR)\jf_mem.c \
    $(JIUTAI_DIR)\jf_process.c

EXTRA_INC_DIR =

EXTRA_LIBS = psapi.lib $(LIB_DIR)\ollogger.lib $(LIB_DIR)\jf_string.lib \
    $(LIB_DIR)\olfiles.lib $(LIB_DIR)\olservmgmt.lib

!include "$(TOPDIR)\mak\winexe.mak"

#-----------------------------------------------------------------------------


