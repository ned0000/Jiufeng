#
#  @file windows.mak
#
#  @brief The main Makefile for cli
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

EXE = olcli.exe

SOURCES = main.c clicmd.c

JIUTAI_SRCS = $(JIUTAI_DIR)\bases.c $(JIUTAI_DIR)\getopt.c \
             $(JIUTAI_DIR)\xmalloc.c

EXTRA_LIBS = $(LIB_DIR)\olclieng.lib $(LIB_DIR)\ollogger.lib \
    $(LIB_DIR)\olstringparse.lib

!include "$(TOPDIR)\mak\winexe.mak"

#-----------------------------------------------------------------------------


