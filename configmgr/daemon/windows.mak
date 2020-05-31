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

SOURCES = ../common/configmgrcommon.c configmgrsetting.c configtree.c configmgr.c main.c

JIUTAI_SRCS = $(JIUTAI_DIR)\getopt.c $(JIUTAI_DIR)\jf_process.c

EXTRA_INC_DIR =

EXTRA_LIBS = psapi.lib $(LIB_DIR)\jf_logger.lib $(LIB_DIR)\jf_string.lib $(LIB_DIR)\jf_jiukun.lib \
    $(LIB_DIR)\olfiles.lib $(LIB_DIR)\jf_servmgmt.lib

!include "$(TOPDIR)\mak\winexe.mak"

#---------------------------------------------------------------------------------------------------


