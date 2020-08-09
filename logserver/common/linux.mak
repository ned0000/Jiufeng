#
#  @file linux.mak
#
#  @brief The Makefile for common object file in log server.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SOURCES = logservercommon.c

EXTRA_CFLAGS =

EXTRA_INC_DIR = -I../../logger

include $(TOPDIR)/mak/lnxobj.mak

#---------------------------------------------------------------------------------------------------


