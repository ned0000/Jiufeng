#
#  @file linux.mak
#
#  @brief The Makefile for cli
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_cli

SOURCES = main.c clicmd.c

JIUTAI_SRCS = 

EXTRA_INC_DIR = 

EXTRA_LIBS = -ljf_logger -ljf_clieng -ljf_string -ljf_jiukun

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


