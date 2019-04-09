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

#-----------------------------------------------------------------------------

SONAME = olxmlparser

SOURCES = xmlparser.c

EXTRA_LIBS = -lolstringparse -ljf_files

JIUTAI_SRCS = jf_mem.c jf_stack.c jf_hashtree.c

EXTRA_INC_DIR = -I../kinc

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

