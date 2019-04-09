#
#  @file windows.make
#
#  @brief the makefile for network interface management library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = jf_ifmgmt
RESOURCE = ifmgmt

SOURCES = ifmgmt.c ipaddr.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c

EXTRA_DEFS = -DJIUFENG_IFMGMT_DLL

EXTRA_LIBS = ws2_32.lib Iphlpapi.lib $(LIB_DIR)\jf_logger.lib

!if "$(DEBUG_JIUFENG)" == "yes"
EXTRA_CFLAGS =
!endif

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



