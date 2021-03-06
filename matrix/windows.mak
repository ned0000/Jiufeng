#
#  @file matrix/windows.mak
#
#  @brief The makefile for matrix library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_matrix

RESOURCE = matrix

SOURCES = matrix.c

JIUTAI_SRCS =

EXTRA_DEFS = /DJIUFENG_MATRIX_DLL

EXTRA_LIBS = jf_logger.lib jf_jiukun.lib

!if "$(DEBUG_JIUFENG)" == "yes"
EXTRA_CFLAGS = $(EXTRA_CFLAGS) /DDEBUG_MATRIX
!endif

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
