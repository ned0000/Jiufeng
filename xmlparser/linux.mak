#
#  @file linux.mak
#
#  @brief the makefile for xml parser library
#
#  @author Min Zhang
#
#  @note
#  
#

#-----------------------------------------------------------------------------

SONAME = olxmlparser

SOURCES = xmlparser.c

EXTRA_LIBS = -lolstringparse -lolfiles

JIUTAI_SRCS = xmalloc.c bases.c

EXTRA_INC_DIR = -I../kinc

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

