#
#  @file windows.mak
#
#  @brief the makefile for HTTP Parser library
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_httpparser
RESOURCE = httpparser

SOURCES = httpparser.c
JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\bases.c

EXTRA_LIBS = $(LIB_DIR)\jf_string.lib $(LIB_DIR)\jf_jiukun.lib

EXTRA_DEFS = -DJIUFENG_HTTPPARSER_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



