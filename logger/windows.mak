#
#  @file windows.mak
#
#  @brief the makefile for logger library on Windows platform
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

DLLNAME = jf_logger
RESOURCE = logger

SOURCES = common.c logger.c errcode.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_hex.c

EXTRA_DEFS = -DJIUFENG_LOGGER_DLL

EXTRA_INC_DIR = 

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



