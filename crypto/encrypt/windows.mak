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

#-----------------------------------------------------------------------------

DLLNAME = olencrypt
RESOURCE = encrypt

SOURCES = encrypt.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c

EXTRA_LIBS = 

EXTRA_DEFS = -DJIUFENG_ENCRYPT_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------


