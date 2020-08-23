#
#  @file logserver/common/windows.make
#
#  @brief The makefile for common object file in log server.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SOURCES = logservercommon.c

EXTRA_CFLAGS =

EXTRA_INC_DIR = /I..\..\logger

!include "$(TOPDIR)\mak\winobj.mak"

#---------------------------------------------------------------------------------------------------
