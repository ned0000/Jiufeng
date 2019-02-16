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

JIUTAI_SRCS = xmalloc.c process.c

EXTRA_INC_DIR = 

EXTRA_LIBS = -lolservmgmt -lolstringparse -lolfiles -lollogger

include $(TOPDIR)/mak/lnxexe.mak

#-----------------------------------------------------------------------------


