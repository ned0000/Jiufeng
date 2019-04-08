#
#  @file windows.make
#
#  @brief the makefile for network library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = olnetwork
RESOURCE = network

SOURCES = internalsocket.c socket.c socketpair.c chain.c \
    utimer.c asocket.c assocket.c acsocket.c \
    adgram.c resolve.c network.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mem.c $(JIUTAI_DIR)\jf_mutex.c \
             $(JIUTAI_DIR)\xtime.c

EXTRA_DEFS = -DJIUFENG_NETWORK_DLL

EXTRA_LIBS = ws2_32.lib Iphlpapi.lib $(LIB_DIR)\ollogger.lib \
             $(LIB_DIR)\olifmgmt.lib $(LIB_DIR)\olstringparse.lib

!if "$(DEBUG_JIUFENG)" == "yes"
    EXTRA_CFLAGS = -DDEBUG_CHAIN -DDEBUG_UTIMER
!endif

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



