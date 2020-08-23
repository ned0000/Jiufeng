#
#  @file clieng/windows.mak
#
#  @brief The main makefile for cli engine.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_clieng
RESOURCE = clieng

SOURCES = clieng.c engio.c cmdparser.c cmdhistory.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mutex.c $(JIUTAI_DIR)\jf_hashtable.c $(JIUTAI_DIR)\jf_hex.c \
    $(JIUTAI_DIR)\jf_process.c

EXTRA_LIBS = ws2_32.lib Psapi.lib jf_logger.lib jf_string.lib jf_files.lib jf_jiukun.lib

EXTRA_DEFS = /DJIUFENG_CLIENG_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
