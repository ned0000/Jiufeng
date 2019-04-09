#
#  @file Makefile
#
#  @brief The Makefile for archive library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SONAME = jf_archive

SOURCES = archive.c arfile.c create.c extract.c

EXTRA_LIBS = -ljf_files -lolstringparse

JIUTAI_SRCS = jf_mem.c jf_linklist.c

EXTRA_INC_DIR = -I../kinc

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

