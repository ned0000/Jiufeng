#
#  @file Makefile
#
#  @brief The Makefile for dongyuan
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

EXE = oldongyuan

SOURCES = dongyuan.c main.c

JIUTAI_SRCS = jf_mem.c process.c

EXTRA_INC_DIR = 

EXTRA_LIBS = -lolservmgmt -lolstringparse -lolfiles -ljf_logger

include $(TOPDIR)/mak/lnxexe.mak

#-----------------------------------------------------------------------------


