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

#---------------------------------------------------------------------------------------------------

SONAME = jf_prng

SOURCES = ../common/clrmem.c prng.c seed.c

JIUTAI_SRCS = jf_mem.c jf_mutex.c jf_time.c

EXTRA_INC_DIR = -I../common

EXTRA_LIBS = -ljf_cghash -ljf_files

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

