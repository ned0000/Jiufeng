#
#  @file windows.mak
#
#  @brief the makefile for messaging library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_messaging

RESOURCE = messaging

SOURCES = messaging.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\jf_process.c $(JIUTAI_DIR)\jf_time.c

EXTRA_DEFS = -DJIUFENG_MESSAGING_DLL

EXTRA_LIBS = psapi.lib $(LIB_DIR)\jf_logger.lib $(LIB_DIR)\olfiles.lib \
    $(LIB_DIR)\jf_ifmgmt.lib $(LIB_DIR)\jf_network.lib
             
EXTRA_INC_DIR = -I../common

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



