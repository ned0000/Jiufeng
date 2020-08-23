#
#  @file logserver/windows.mak
#
#  @brief The main Makefile for log server control utility and daemon.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SUBDIRS = common daemon logctl

!include "$(TOPDIR)\mak\winsubdirs.mak"

#---------------------------------------------------------------------------------------------------
