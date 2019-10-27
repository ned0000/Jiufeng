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

#---------------------------------------------------------------------------------------------------

SONAME = jf_matrix

SOURCES = matrix.c

JIUTAI_SRCS =

EXTRA_LIBS = -ljf_logger -ljf_jiukun

ifeq ("$(DEBUG_JIUFENG)", "yes")
    EXTRA_CFLAGS = -DDEBUG_MATRIX
endif

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

