#
#  @file linux.mak
#
#  @brief The makefile for messaging library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_messaging

SOURCES = messaging.c

JIUTAI_SRCS = jf_mem.c jf_process.c jf_time.c jf_mutex.c

EXTRA_LIBS = -ljf_logger -ljf_files -ljf_ifmgmt -ljf_network

EXTRA_INC_DIR = -I../common

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

