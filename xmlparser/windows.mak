#
#  @file xmlparser/windows.mak
#
#  @brief The makefile for XML Parser library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_xmlparser
RESOURCE = xmlparser

SOURCES = xmlcommon.c xmlattr.c xmlparser.c xmlprint.c xmlfile.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_stack.c $(JIUTAI_DIR)\jf_hashtree.c $(JIUTAI_DIR)\jf_linklist.c \
    $(JIUTAI_DIR)\jf_ptree.c

EXTRA_LIBS = jf_string.lib jf_files.lib jf_jiukun.lib jf_logger.lib

EXTRA_INC_DIR =

EXTRA_DEFS = /DJIUFENG_XMLPARSER_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
