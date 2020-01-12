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

SOURCES = ../common/dispatchercommon.c messagingserver.c messagingclient.c messaging.c

JIUTAI_SRCS = jf_process.c jf_time.c jf_mutex.c jf_thread.c

EXTRA_LIBS = -ljf_logger -ljf_files -ljf_ifmgmt -ljf_network -ljf_jiukun -ljf_dispatcher_xfer

EXTRA_INC_DIR = -I../common -I../xfer

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

