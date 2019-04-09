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

SONAME = jf_matrix

SOURCES = matrix.c

JIUTAI_SRCS = jf_mem.c

EXTRA_LIBS = -ljf_logger

ifeq ("$(DEBUG_JIUFENG)", "yes")
EXTRA_CFLAGS = -DDEBUG_MATRIX
endif

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

