#
#  @file Makefile
#
#  @brief The Makefile for dispatcher service
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_dispatcher

SOURCES = servconfig.c dispatcher.c main.c

JIUTAI_SRCS = jf_process.c jf_mutex.c jf_thread.c jf_user.c jf_queue.c jf_ptree.c jf_linklist.c \
    jf_hashtree.c jf_stack.c jf_option.c

EXTRA_LIBS = -ljf_string -ljf_files -ljf_logger -ljf_ifmgmt -ljf_network -ljf_jiukun -ljf_xmlparser

EXTRA_INC_DIR = -I../common

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


