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

DLLNAME = jf_uuid
RESOURCE = uuid

SOURCES = uuid.c output.c
JIUTAI_SRCS = $(JIUTAI_DIR)\jf_time.c $(JIUTAI_DIR)\jf_mem.c

EXTRA_LIBS = $(LIB_DIR)\jf_ifmgmt.lib $(LIB_DIR)\jf_cghash.lib \
    $(LIB_DIR)\olprng.lib

EXTRA_DEFS = -DJIUFENG_UUID_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



