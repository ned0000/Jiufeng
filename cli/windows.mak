#
#  @file cli/windows.mak
#
#  @brief The main Makefile for cli.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_cli.exe

SOURCES = main.c clicmd.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_option.c

EXTRA_LIBS = jf_clieng.lib jf_logger.lib jf_jiukun.lib jf_string.lib jf_files.lib

!include "$(TOPDIR)\mak\winexe.mak"

#---------------------------------------------------------------------------------------------------


