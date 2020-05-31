#
#  @file configmgr/configctl/linux.mak
#
#  @brief The Makefile for config control utility.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_configctl

SOURCES = configctl.c

JIUTAI_SRCS = jf_option.c

EXTRA_LIBS = -ljf_jiukun -ljf_config -ljf_logger -ljf_files

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


