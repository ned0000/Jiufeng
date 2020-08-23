#
#  @file cli/linux.mak
#
#  @brief The Makefile for cli.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_cli

SOURCES = main.c clicmd.c

JIUTAI_SRCS = jf_option.c

EXTRA_INC_DIR = 

EXTRA_LIBS = -ljf_logger -ljf_clieng -ljf_string -ljf_jiukun -ljf_files

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


