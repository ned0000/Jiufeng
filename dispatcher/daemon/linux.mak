#
#  @file Makefile
#
#  @brief The Makefile for dispatcher service.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_dispatcher

SOURCES = ../common/dispatchercommon.c servconfig.c servclient.c servserver.c dispatcher.c main.c

JIUTAI_SRCS = jf_process.c jf_mutex.c jf_thread.c jf_user.c jf_queue.c jf_ptree.c jf_linklist.c \
    jf_hashtree.c jf_stack.c jf_option.c jf_sem.c

EXTRA_LIBS = -ljf_string -ljf_files -ljf_logger -ljf_ifmgmt -ljf_network -ljf_jiukun \
    -ljf_xmlparser -ljf_dispatcher_xfer

EXTRA_INC_DIR = -I../common -I../xfer

EXTRA_CFLAGS = -D_GNU_SOURCE

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


