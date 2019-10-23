#
#  @file windows.mak
#
#  @brief the makefile for service library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_serv

RESOURCE = serv

SOURCES = ../common/servmgmtcommon.c serv.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\jf_process.c $(JIUTAI_DIR)\jf_time.c

EXTRA_DEFS = -DJIUFENG_SERV_DLL

EXTRA_LIBS = psapi.lib $(LIB_DIR)\jf_logger.lib $(LIB_DIR)\jf_files.lib $(LIB_DIR)\jf_jiukun.lib \
    $(LIB_DIR)\jf_ifmgmt.lib $(LIB_DIR)\jf_network.lib
             
EXTRA_INC_DIR = -I../common

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



