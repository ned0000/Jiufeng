#
#  @file string/windows.mak
#
#  @brief The makefile for string library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_string
RESOURCE = string

SOURCES = parsestring.c scanstring.c printstring.c validatestring.c settingparse.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_time.c $(JIUTAI_DIR)\jf_date.c $(JIUTAI_DIR)\jf_option.c

EXTRA_LIBS = ws2_32.lib jf_jiukun.lib

EXTRA_DEFS = /DJIUFENG_STRING_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
