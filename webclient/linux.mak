#
#  @file linux.mak
#
#  @brief the makefile for webclient library
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_webclient

SOURCES = webclient.c

JIUTAI_SRCS = jf_hashtree.c jf_queue.c jf_mutex.c jf_hex.c

EXTRA_LIBS = -ljf_httpparser -ljf_network -ljf_string -ljf_jiukun -ljf_logger

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

