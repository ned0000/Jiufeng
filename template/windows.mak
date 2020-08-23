#
#  @file
#
#  @brief The template makefile for Windows.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = oltemplate
RESOURCE = template

SOURCES = template.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_hex.c

EXTRA_DEFS = -DJIUFENG_TEMPLATE_DLL

EXTRA_LIBS =

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
