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

SOURCES = ../servmgmtcommon/servmgmtcommon.c serv.c

JIUTAI_SRCS = jf_mem.c jf_process.c jf_time.c

EXTRA_LIBS = -ljf_logger -ljf_files -ljf_ifmgmt -ljf_network

EXTRA_INC_DIR = -I../servmgmtcommon

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

