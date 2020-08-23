#
#  @file encode/linux.mak
#
#  @brief The Makefile for encode-decode library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_encode

SOURCES = base64.c huffman.c

JIUTAI_SRCS =

EXTRA_LIBS = -ljf_jiukun

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------
