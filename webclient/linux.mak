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

JIUTAI_SRCS = bases.c jf_mem.c jf_mutex.c hexstr.c

EXTRA_LIBS = -lolhttpparser -lolnetwork -lolstringparse

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

