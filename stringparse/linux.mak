#
#  @file linux.mak
#
#  @brief the makefile for stringparse library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

SONAME = olstringparse

JIUTAI_SRCS = jf_mem.c xtime.c

SOURCES = parsestring.c scanstring.c printstring.c validatestring.c \
    settingparse.c

EXTRA_LIBS =

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

