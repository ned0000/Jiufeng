#
#  @file Makefile
#
#  @brief The Makefile for network interface management library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SONAME = olifmgmt

SOURCES = ifmgmt.c ipaddr.c

JIUTAI_SRCS = xmalloc.c

EXTRA_LIBS = -lolfiles -lolstringparse

ifeq ("$(DEBUG_JIUFENG)", "yes")
    EXTRA_CFLAGS =
endif

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

