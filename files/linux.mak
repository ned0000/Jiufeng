#
#  @file linux.mak
#
#  @brief the makefile for files library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

SONAME = olfiles

SOURCES = files.c directory.c conffile.c

EXTRA_LIBS =

JIUTAISRCS = xmalloc.c

EXTRA_INC_DIR =

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

