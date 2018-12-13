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

JIUTAI_SRCS = xmalloc.c

EXTRA_LIBS = -lssl -lcrypt -lolstringparse -lolfiles

SONAME = olencrypt

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

