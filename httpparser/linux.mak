#
#  @file linux.mak
#
#  @brief the makefile for HTTP parser library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SONAME = olhttpparser

SOURCES = httpparser.c

EXTRA_LIBS = -lolstringparse

JIUTAI_SRCS = xmalloc.c bases.c

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

