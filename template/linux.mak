#
#  @file
#
#  @brief the template makefile for Linux
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SONAME = oltemplate

SOURCES = template.c

JIUTAI_SRCS = hexstr.c

EXTRA_LIBS = 

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

