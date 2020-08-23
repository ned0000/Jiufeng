#
#  @file logserver/logctl/windows.mak
#
#  @brief The main makefile for log control utility.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_logctl.exe

SOURCES = ..\common\logservercommon.c logctl.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_option.c

EXTRA_INC_DIR = /I..\..\logger /I..\common

EXTRA_LIBS = jf_logger.lib jf_jiukun.lib jf_files.lib

!include "$(TOPDIR)\mak\winexe.mak"

#---------------------------------------------------------------------------------------------------
