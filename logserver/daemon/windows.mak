#
#  @file logserver/daemon/windows.mak
#
#  @brief The main makefile for log server.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_logserver.exe

SOURCES = ..\..\logger\common.c ..\..\logger\log2stdout.c ..\..\logger\log2file.c \
    ..\common\logservercommon.c logsave.c logserver.c main.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_process.c $(JIUTAI_DIR)\jf_mutex.c $(JIUTAI_DIR)\jf_thread.c \
    $(JIUTAI_DIR)\jf_option.c $(JIUTAI_DIR)\jf_time.c $(JIUTAI_DIR)\jf_mem.c

EXTRA_INC_DIR = /I..\..\logger /I..\common

EXTRA_LIBS = ws2_32.lib psapi.lib jf_logger.lib jf_string.lib jf_jiukun.lib jf_files.lib \
    jf_ifmgmt.lib jf_network.lib

!include "$(TOPDIR)\mak\winexe.mak"

#---------------------------------------------------------------------------------------------------
