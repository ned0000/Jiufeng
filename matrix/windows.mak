#
#  @file windows.mak
#
#  @brief The makefile for matrix library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = olmatrix

RESOURCE = matrix

SOURCES = matrix.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c

EXTRA_DEFS = -DJIUFENG_MATRIX_DLL

EXTRA_LIBS = jf_logger.lib

!if "$(DEBUG_JIUFENG)" == "yes"
EXTRA_CFLAGS = -DDEBUG_MATRIX
!endif

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



