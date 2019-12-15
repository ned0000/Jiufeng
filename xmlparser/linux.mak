#
#  @file linux.mak
#
#  @brief the makefile for xml parser library
#
#  @author Min Zhang
#
#  @note
#  
#

#---------------------------------------------------------------------------------------------------

SONAME = jf_xmlparser

SOURCES = xmlcommon.c xmlattr.c xmlparser.c xmlprint.c xmlfile.c

JIUTAI_SRCS = jf_stack.c jf_hashtree.c jf_linklist.c

EXTRA_LIBS = -ljf_string -ljf_files -ljf_jiukun

EXTRA_INC_DIR = -I../kinc

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

