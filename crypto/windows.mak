#
#  @file crypto/windows.mak
#
#  @brief The main Makefile for libraries related to cryptography.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SUBDIRS = common cghash cgmac encrypt prng

!include "$(TOPDIR)\mak\winsubdirs.mak"

#---------------------------------------------------------------------------------------------------
