#
#  @file linux.mak
#
#  @brief the makefile for files library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_files

SOURCES = files.c directory.c conffile.c

EXTRA_LIBS = -ljf_jiukun

JIUTAI_SRCS = jf_option.c

EXTRA_INC_DIR =

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

