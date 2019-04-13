#
#  @file windows.mak
#
#  @brief The main Makefile for cli
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = olcli.exe

SOURCES = main.c clicmd.c

JIUTAI_SRCS = $(JIUTAI_DIR)\bases.c $(JIUTAI_DIR)\getopt.c \
             $(JIUTAI_DIR)\jf_mem.c

EXTRA_LIBS = $(LIB_DIR)\jf_clieng.lib $(LIB_DIR)\jf_logger.lib \
    $(LIB_DIR)\jf_string.lib

!include "$(TOPDIR)\mak\winexe.mak"

#---------------------------------------------------------------------------------------------------


