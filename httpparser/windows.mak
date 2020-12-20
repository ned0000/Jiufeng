#
#  @file httpparser/windows.mak
#
#  @brief The makefile for HTTP Parser library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_httpparser
RESOURCE = httpparser

SOURCES = httpparser.c chunkprocessor.c dataobject.c

JIUTAI_SRCS =

EXTRA_LIBS = jf_string.lib jf_jiukun.lib jf_logger.lib

EXTRA_DEFS = /DJIUFENG_HTTPPARSER_DLL

!if "$(DEBUG_JIUFENG)" == "yes"
#EXTRA_CFLAGS = $(EXTRA_CFLAGS) /DDEBUG_HTTPPARSER
!endif

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
