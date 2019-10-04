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

SOURCES = ../servmgmtcommon/servmgmtcommon.c servmgmtsetting.c servmgmt.c dongyuan.c main.c

JIUTAI_SRCS = jf_mem.c jf_process.c jf_mutex.c jf_attask.c jf_thread.c

EXTRA_INC_DIR = ../servmgmtcommon

EXTRA_LIBS = -ljf_string -ljf_files -ljf_logger -ljf_ifmgmt -ljf_network -lxml2

EXTRA_INC_DIR = -I/usr/include/libxml2 -I../servmgmtcommon

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


