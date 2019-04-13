#
#  @file linux.mak
#
#  @brief the makefile for HTTP parser library
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_httpparser

SOURCES = httpparser.c

EXTRA_LIBS = -ljf_string

JIUTAI_SRCS = jf_mem.c 

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

