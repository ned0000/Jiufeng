#
#  @file logserver/logctl/linux.mak
#
#  @brief The Makefile for log control utility.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_logctl

SOURCES = ../common/logservercommon.c logctl.c

JIUTAI_SRCS = jf_option.c

EXTRA_LIBS = -ljf_jiukun -ljf_logger -ljf_files

EXTRA_INC_DIR = -I../../logger -I../common

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


