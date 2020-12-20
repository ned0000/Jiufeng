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

ifeq ("$(DEBUG_JIUFENG)", "yes")
#  EXTRA_CFLAGS += -DDEBUG_HTTPPARSER
endif

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------
