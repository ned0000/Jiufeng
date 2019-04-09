#
#  @file windows.mak
#
#  @brief The main Makefile for hash function library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = jf_cghash
RESOURCE = cghash

SOURCES = md5.c sha1.c

JIUTAI_SRCS = 

EXTRA_LIBS = 

EXTRA_DEFS = -DJIUFENG_CGHASH_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------


