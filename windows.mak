#
#  @file windows.mak
#
#  @brief The main makefile for Jiufeng.
#
#  @author Min Zhang
#
#  @note
#  -# Set environment variable "TOPDIR" first.
#

#---------------------------------------------------------------------------------------------------

!if "$(TOPDIR)" == ""

!message Environment variable TOPDIR is not set.

!else

SUBDIRS = jiutai logger jiukun test

!include "$(TOPDIR)\mak\winsubdirs.mak"

!endif

#---------------------------------------------------------------------------------------------------



