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

#-----------------------------------------------------------------------------

SOURCES = encrypt.c

JIUTAI_SRCS = jf_mem.c

EXTRA_LIBS = -lssl -lcrypt -lolstringparse -lolfiles

SONAME = olencrypt

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

