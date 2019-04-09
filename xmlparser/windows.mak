#
#  @file windows.mak
#
#  @brief the makefile for XML Parser library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

DLLNAME = olxmlparser
RESOURCE = xmlparser

SOURCES = xmlparser.c
JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\bases.c

EXTRA_LIBS = $(LIB_DIR)\jf_string.lib $(LIB_DIR)\olfiles.lib

EXTRA_INC_DIR =

EXTRA_DEFS = -DJIUFENG_XMLPARSER_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



