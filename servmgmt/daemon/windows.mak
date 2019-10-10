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

#---------------------------------------------------------------------------------------------------

EXE = jf_dongyuan.exe

SOURCES = dongyuan.c main.c

JIUTAI_SRCS = $(JIUTAI_DIR)\getopt.c $(JIUTAI_DIR)\jf_mem.c \
    $(JIUTAI_DIR)\jf_process.c

EXTRA_INC_DIR =

EXTRA_LIBS = psapi.lib $(LIB_DIR)\jf_logger.lib $(LIB_DIR)\jf_string.lib \
    $(LIB_DIR)\olfiles.lib $(LIB_DIR)\jf_servmgmt.lib

!include "$(TOPDIR)\mak\winexe.mak"

#---------------------------------------------------------------------------------------------------


