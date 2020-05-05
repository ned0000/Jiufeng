#
#  @file linux.mak
#
#  @brief The Makefile for common object file in dispatcher
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SOURCES = dispatchercommon.c prioqueue.c

EXTRA_CFLAGS =

include $(TOPDIR)/mak/lnxobj.mak

#---------------------------------------------------------------------------------------------------


