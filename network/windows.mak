#
#  @file network/windows.make
#
#  @brief The makefile for network library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_network
RESOURCE = network

SOURCES = internalsocket.c socket.c socketpair.c chain.c utimer.c asocket.c assocket.c acsocket.c \
    adgram.c resolve.c transfer.c network.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mutex.c $(JIUTAI_DIR)\jf_time.c

EXTRA_DEFS = /DJIUFENG_NETWORK_DLL

EXTRA_LIBS = ws2_32.lib Iphlpapi.lib jf_logger.lib jf_jiukun.lib jf_ifmgmt.lib

!if "$(DEBUG_JIUFENG)" == "yes"
#EXTRA_CFLAGS += $(EXTRA_CFLAGS) /DDEBUG_CHAIN
#EXTRA_CFLAGS += $(EXTRA_CFLAGS) /DDEBUG_UTIMER
#EXTRA_CFLAGS += $(EXTRA_CFLAGS) /DDEBUG_ASOCKET
!endif

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
