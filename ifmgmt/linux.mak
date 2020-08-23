#
#  @file ifmgmt/linux.mak
#
#  @brief The Makefile for network interface management library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_ifmgmt

SOURCES = ifmgmt.c ipaddr.c

JIUTAI_SRCS = 

EXTRA_LIBS = -ljf_files -ljf_string

ifeq ("$(DEBUG_JIUFENG)", "yes")
    EXTRA_CFLAGS =
endif

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------
