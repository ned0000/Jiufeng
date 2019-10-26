#
#  @file windows.mak
#
#  @brief The main Makefile for encrypt library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_encrypt
RESOURCE = encrypt

SOURCES = encrypt.c

JIUTAI_SRCS = 

EXTRA_LIBS = 

EXTRA_DEFS = -DJIUFENG_ENCRYPT_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------


