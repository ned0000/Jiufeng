#
#  @file windows.mak
#
#  @brief the makefile for stringparse library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = jf_string
RESOURCE = stringparse

SOURCES = parsestring.c scanstring.c printstring.c validatestring.c \
     settingparse.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\jf_time.c $(JIUTAI_DIR)\jf_date.c

EXTRA_LIBS = ws2_32.lib

EXTRA_DEFS = -DJIUFENG_STRINGPARSE_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



