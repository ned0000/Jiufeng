#
#  @file files/linux.mak
#
#  @brief The makefile for files library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_files

SOURCES = file.c filestream.c directory.c conffile.c

EXTRA_LIBS = -ljf_jiukun -ljf_string

JIUTAI_SRCS = jf_option.c

EXTRA_INC_DIR =

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------
