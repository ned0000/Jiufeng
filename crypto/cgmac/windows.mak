#
#  @file crypto/cgmac/windows.mak
#
#  @brief The main Makefile for cryptographic message authentication code.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_cgmac
RESOURCE = cgmac

SOURCES = hmac.c

JIUTAI_SRCS = 

EXTRA_LIBS = $(LIB_DIR)\jf_cghash.lib

EXTRA_INC_DIR = /I../cghash

EXTRA_DEFS = /DJIUFENG_CGMAC_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
