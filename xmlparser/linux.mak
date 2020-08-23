#
#  @file xmlparser/linux.mak
#
#  @brief The makefile for xml parser library.
#
#  @author Min Zhang
#
#  @note
#  
#

#---------------------------------------------------------------------------------------------------

SONAME = jf_xmlparser

SOURCES = xmlcommon.c xmlattr.c xmlparser.c xmlprint.c xmlfile.c

JIUTAI_SRCS = jf_stack.c jf_hashtree.c jf_linklist.c jf_ptree.c

EXTRA_LIBS = -ljf_string -ljf_files -ljf_jiukun

EXTRA_INC_DIR =

ifeq ("$(DEBUG_JIUFENG)", "yes")
#    EXTRA_CFLAGS += -DDEBUG_XML_DOC
endif

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------
