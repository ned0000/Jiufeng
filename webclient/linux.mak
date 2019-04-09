#
#  @file linux.mak
#
#  @brief the makefile for webclient library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SONAME = olwebclient

SOURCES = webclient.c

JIUTAI_SRCS = jf_hashtree.c jf_queue.c jf_mem.c jf_mutex.c hexstr.c

EXTRA_LIBS = -lolhttpparser -ljf_network -ljf_string

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

