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

EXTRA_LIBS = -ljf_jiukun -ljf_serv -ljf_logger -ljf_files -ljf_ifmgmt -ljf_network -ljf_string

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


