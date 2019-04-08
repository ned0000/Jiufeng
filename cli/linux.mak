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

#-----------------------------------------------------------------------------

EXE = olcli

SOURCES = main.c clicmd.c

JIUTAI_SRCS = 

EXTRA_INC_DIR = 

EXTRA_LIBS = -ljf_logger -lolclieng -lolstringparse

include $(TOPDIR)/mak/lnxexe.mak

#-----------------------------------------------------------------------------


