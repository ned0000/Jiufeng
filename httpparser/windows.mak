#
#  @file windows.mak
#
#  @brief the makefile for HTTP Parser library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

DLLNAME = olhttpparser
RESOURCE = httpparser

SOURCES = httpparser.c
JIUTAI_SRCS = $(JIUTAI_DIR)\xmalloc.c $(JIUTAI_DIR)\bases.c

EXTRA_LIBS = $(LIB_DIR)\olstringparse.lib

EXTRA_DEFS = -DJIUFENG_HTTPPARSER_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



