#
#  @file windows.mak
#
#  @brief the makefile for UUID library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

DLLNAME = oluuid
RESOURCE = uuid

SOURCES = uuid.c output.c
JIUTAI_SRCS = $(JIUTAI_DIR)\xtime.c $(JIUTAI_DIR)\xmalloc.c

EXTRA_LIBS = $(LIB_DIR)\olifmgmt.lib $(LIB_DIR)\olcghash.lib \
    $(LIB_DIR)\olprng.lib

EXTRA_DEFS = -DJIUFENG_UUID_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



