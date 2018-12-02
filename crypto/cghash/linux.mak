#
#  @file linux.mak
#
#  @brief The Makefile for hash function library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SOURCES = md5.c sha1.c

JIUTAI_SRCS = 

EXTRA_LIBS =

SONAME = olcghash

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

