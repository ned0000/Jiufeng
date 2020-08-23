#
#  @file httpparser/linux.mak
#
#  @brief The makefile for HTTP parser library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_httpparser

SOURCES = httpparser.c chunkprocessor.c dataobject.c

EXTRA_LIBS = -ljf_string -ljf_jiukun -ljf_logger

JIUTAI_SRCS =

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------
