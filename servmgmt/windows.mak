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

DLLNAME = jf_servmgmt

RESOURCE = servmgmt

SOURCES = servmgmt.c servmgmtsetting.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\sharedmemory.c \
    $(JIUTAI_DIR)\jf_process.c $(JIUTAI_DIR)\attask.c $(JIUTAI_DIR)\jf_time.c

EXTRA_DEFS = -DJIUFENG_SERVMGMT_DLL

EXTRA_LIBS = psapi.lib $(LIB_DIR)\jf_logger.lib $(LIB_DIR)\olfiles.lib \
    $(LIB_DIR)\jf_xmlparser.lib $(LIB_DIR)\jf_uuid.lib
             
EXTRA_INC_DIR =

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



