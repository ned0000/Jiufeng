#
#  @file windows.mak
#
#  @brief The main Makefile for archive library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = olarchive
RESOURCE = archive

SOURCES = archive.c arfile.c create.c extract.c
JIUTAI_SRCS = $(JIUTAI_DIR)\bases.c $(JIUTAI_DIR)\xmalloc.c 

EXTRA_LIBS = $(LIB_DIR)\ollogger.lib $(LIB_DIR)\olfiles.lib \
             $(LIB_DIR)\olstringparse.lib

EXTRA_INC_DIR = -I..\kinc

EXTRA_DEFS = -DJIUFENG_ARCHIVE_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------


