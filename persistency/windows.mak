#
#  @file persistency/windows.mak
#
#  @brief The makefile for persistency library on Windows platform.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_persistency
RESOURCE = persistency

SOURCES = persistencycommon.c filepersistency.c persistency.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mutex.c $(JIUTAI_DIR)\jf_hashtree.c

EXTRA_DEFS = /DJIUFENG_PERSISTENCY_DLL

EXTRA_INC_DIR = 

EXTRA_LIBS = jf_jiukun.lib jf_logger.lib jf_string.lib jf_files.lib

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
