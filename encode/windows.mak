#
#  @file encode/windows.mak
#
#  @brief The makefile for encode-decode library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_encode
RESOURCE = encode

SOURCES = base64.c huffman.c

JIUTAI_SRCS =

EXTRA_LIBS = $(LIB_DIR)\jf_jiukun.lib

EXTRA_DEFS = /DJIUFENG_ENCODE_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



