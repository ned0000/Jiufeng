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

#---------------------------------------------------------------------------------------------------

SONAME = jf_encode

SOURCES = base64.c huffman.c

JIUTAI_SRCS = jf_mem.c

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

