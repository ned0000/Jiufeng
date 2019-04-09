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

JIUTAI_SRCS = jf_mem.c jf_process.c

EXTRA_INC_DIR = 

EXTRA_LIBS = -lolservmgmt -lolstringparse -ljf_files -ljf_logger

include $(TOPDIR)/mak/lnxexe.mak

#-----------------------------------------------------------------------------


