#
#  @file windows.mak
#
#  @brief the makefile for XML Parser library
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_xmlparser
RESOURCE = xmlparser

SOURCES = xmlparser.c
JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\jf_stack.c $(JIUTAI_DIR)\jf_hashtree.c

EXTRA_LIBS = $(LIB_DIR)\jf_string.lib $(LIB_DIR)\jf_files.lib $(LIB_DIR)\jf_jiukun.lib

EXTRA_INC_DIR =

EXTRA_DEFS = -DJIUFENG_XMLPARSER_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



