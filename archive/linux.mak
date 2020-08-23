#
#  @file archive/linux.mak
#
#  @brief The Makefile for archive library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_archive

SOURCES = archive.c arfile.c create.c extract.c

EXTRA_LIBS = -ljf_files -ljf_string -ljf_jiukun

JIUTAI_SRCS = jf_linklist.c

EXTRA_INC_DIR =

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------
