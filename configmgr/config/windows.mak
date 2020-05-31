#
#  @file windows.mak
#
#  @brief the makefile for config library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_config

RESOURCE = config

SOURCES = ../common/configmgrcommon.c config.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_process.c $(JIUTAI_DIR)\jf_time.c

EXTRA_DEFS = -DJIUFENG_CONFIG_DLL

EXTRA_LIBS = psapi.lib $(LIB_DIR)\jf_logger.lib $(LIB_DIR)\jf_files.lib $(LIB_DIR)\jf_jiukun.lib \
    $(LIB_DIR)\jf_ifmgmt.lib $(LIB_DIR)\jf_network.lib
             
EXTRA_INC_DIR = -I../common

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



