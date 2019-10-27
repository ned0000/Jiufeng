#
#  @file Makefile
#
#  @brief The Makefile for dongyuan
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_dongyuan

CONFIG_FILES = servmgmt.setting

SOURCES = ../common/servmgmtcommon.c servmgmtsetting.c servmgmt.c dongyuan.c main.c

JIUTAI_SRCS = jf_process.c jf_mutex.c jf_attask.c jf_thread.c jf_sharedmemory.c

EXTRA_LIBS = -ljf_string -ljf_files -ljf_logger -ljf_ifmgmt -ljf_network -lxml2 -ljf_jiukun

EXTRA_INC_DIR = -I/usr/include/libxml2 -I../common

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


