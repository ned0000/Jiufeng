#
#  @file uuid/windows.mak
#
#  @brief The makefile for UUID library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_uuid
RESOURCE = uuid

SOURCES = uuid.c output.c
JIUTAI_SRCS = $(JIUTAI_DIR)\jf_time.c

EXTRA_LIBS = jf_ifmgmt.lib jf_cghash.lib jf_prng.lib

EXTRA_DEFS = /DJIUFENG_UUID_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
