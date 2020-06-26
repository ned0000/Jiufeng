#
#  @file linux.mak
#
#  @brief The makefile for stringparse library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_string

JIUTAI_SRCS = jf_time.c jf_date.c jf_option.c

SOURCES = parsestring.c scanstring.c printstring.c validatestring.c settingparse.c

EXTRA_LIBS = -ljf_jiukun

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

