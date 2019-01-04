#
#  @file linux.mak
#
#  @brief The Makefile for encode-decode library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

SONAME = olencode

SOURCES = base64.c huffman.c

JIUTAI_SRCS = xmalloc.c

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

