#
#  @file
#
#  @brief the template makefile for Windows
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

DLLNAME = oltemplate
RESOURCE = template

SOURCES = template.c

JIUTAI_SRCS = $(JIUTAI_DIR)\hexstr.c

EXTRA_DEFS = -DJIUFENG_TEMPLATE_DLL

EXTRA_LIBS =

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------
# $Log$


