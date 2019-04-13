#
#  @file linux.mak
#
#  @brief The Makefile for hash function library
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SOURCES = md5.c sha1.c

JIUTAI_SRCS = 

EXTRA_LIBS =

SONAME = jf_cghash

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

