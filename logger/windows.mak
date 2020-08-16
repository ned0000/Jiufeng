#
#  @file logger\windows.mak
#
#  @brief The makefile for logger library on Windows platform.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_logger
RESOURCE = logger

SOURCES = common.c log2stdout.c log2systemlog.c log2file.c log2server.c logger.c errcode.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_hex.c $(JIUTAI_DIR)\jf_time.c $(JIUTAI_DIR)\jf_mem.c

EXTRA_DEFS = -DJIUFENG_LOGGER_DLL

EXTRA_INC_DIR = 

EXTRA_LIBS = ws2_32.lib # Iphlpapi.lib

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------



