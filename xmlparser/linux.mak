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

SOURCES = xmlparser.c

JIUTAI_SRCS = jf_mem.c jf_stack.c jf_hashtree.c

EXTRA_LIBS = -ljf_string -ljf_files -ljf_jiukun

EXTRA_INC_DIR = -I../kinc

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

