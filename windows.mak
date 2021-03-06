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

SUBDIRS = jiutai logger jiukun string files ifmgmt crypto encode uuid  \
    persistency archive xmlparser httpparser network webclient clieng  \
    matrix logserver cli utility test

!include "$(TOPDIR)\mak\winsubdirs.mak"

!endif

#---------------------------------------------------------------------------------------------------



