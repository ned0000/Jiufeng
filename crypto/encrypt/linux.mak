#
#  @file linux.mak
#
#  @brief The Makefile for encrypt library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SOURCES = encrypt.c

JIUTAI_SRCS = jf_hex.c

EXTRA_LIBS = -lssl -lcrypt -ljf_string -ljf_files

SONAME = jf_encrypt

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

