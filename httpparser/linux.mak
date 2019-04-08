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

JIUTAI_SRCS = jf_mem.c 

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

