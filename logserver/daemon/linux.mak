#
#  @file Makefile
#
#  @brief The Makefile for log server.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_logserver

CONFIG_FILES = 

SOURCES = ../../logger/common.c ../../logger/log2tty.c ../../logger/log2stdout.c \
    ../../logger/log2file.c ../common/logservercommon.c logsave.c logserver.c main.c

JIUTAI_SRCS = jf_process.c jf_mutex.c jf_thread.c jf_option.c jf_queue.c jf_mem.c

EXTRA_LIBS = -ljf_string -ljf_files -ljf_logger -ljf_ifmgmt -ljf_network -ljf_jiukun

EXTRA_INC_DIR = -I../../logger -I../common

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


