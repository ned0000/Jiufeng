#
#  @file windows.mak
#
#  @brief the makefile for service management library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = olservmgmt

RESOURCE = servmgmt

SOURCES = servmgmt.c servmgmtsetting.c

JIUTAI_SRCS = $(JIUTAI_DIR)\xmalloc.c $(JIUTAI_DIR)\sharedmemory.c \
    $(JIUTAI_DIR)\process.c $(JIUTAI_DIR)\attask.c $(JIUTAI_DIR)\xtime.c

EXTRA_DEFS = -DJIUFENG_SERVMGMT_DLL

EXTRA_LIBS = psapi.lib $(LIB_DIR)\ollogger.lib $(LIB_DIR)\olfiles.lib \
    $(LIB_DIR)\olxmlparser.lib $(LIB_DIR)\oluuid.lib
             
EXTRA_INC_DIR =

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



