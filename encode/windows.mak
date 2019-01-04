#
#  @file windows.mak
#
#  @brief the makefile for encode-decode library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = olencode
RESOURCE = encode

SOURCES = base64.c huffman.c

JIUTAI_SRCS = $(JIUTAI_DIR)\xmalloc.c

EXTRA_LIBS =

EXTRA_DEFS = -DJIUFENG_ENCODE_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



