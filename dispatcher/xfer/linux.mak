#
#  @file linux.mak
#
#  @brief The makefile for dispatcher xfer library
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_dispatcher_xfer

SOURCES = ../common/dispatchercommon.c xferpool.c dispatcherxfer.c

JIUTAI_SRCS = jf_hashtree.c jf_queue.c jf_mutex.c jf_hex.c jf_hsm.c

EXTRA_LIBS = -ljf_network -ljf_string -ljf_jiukun -ljf_logger

EXTRA_INC_DIR = -I../common

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

