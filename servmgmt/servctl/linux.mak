#
#  @file servmgmt/servctl/linux.mak
#
#  @brief The Makefile for service control utility.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_servctl

SOURCES = servctl.c

JIUTAI_SRCS = jf_option.c

EXTRA_LIBS = -ljf_jiukun -ljf_serv -ljf_logger

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


