#
#  @file linux.mak
#
#  @brief The makefile for matrix library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

SONAME = olmatrix

SOURCES = matrix.c

JIUTAI_SRCS = xmalloc.c

EXTRA_LIBS = -ljf_logger

ifeq ("$(DEBUG_JIUFENG)", "yes")
EXTRA_CFLAGS = -DDEBUG_MATRIX
endif

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

