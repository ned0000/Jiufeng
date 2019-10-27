#
#  @file linux.mak
#
#  @brief the makefile for service library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_serv

SOURCES = ../common/servmgmtcommon.c serv.c

JIUTAI_SRCS = jf_process.c jf_time.c jf_mutex.c jf_sharedmemory.c

EXTRA_LIBS = -ljf_logger -ljf_files -ljf_ifmgmt -ljf_network -ljf_jiukun

EXTRA_INC_DIR = -I../common

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

