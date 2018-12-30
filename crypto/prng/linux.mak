#
#  @file linux.mak
#
#  @brief The makefile for pseudo random number generator library
#
#  @author Min Zhang
#
#  @note
#  
#

#-----------------------------------------------------------------------------

SONAME = olprng

SOURCES = ../common/clrmem.c prng.c seed.c

JIUTAI_SRCS = xmalloc.c syncmutex.c xtime.c

EXTRA_INC_DIR = -I../common

EXTRA_LIBS = -lolcghash -lolfiles

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

