#
#  @file webclient/windows.mak
#
#  @brief The makefile for webclient library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_webclient
RESOURCE = webclient

SOURCES = common.c webclientrequest.c dataobjectpool.c webclient.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_hashtree.c $(JIUTAI_DIR)\jf_queue.c $(JIUTAI_DIR)\jf_mutex.c \
    $(JIUTAI_DIR)\jf_time.c $(JIUTAI_DIR)\jf_hsm.c

EXTRA_LIBS = ws2_32.lib jf_logger.lib jf_ifmgmt.lib jf_jiukun.lib jf_httpparser.lib \
    jf_network.lib jf_string.lib

EXTRA_DEFS = /DJIUFENG_WEBCLIENT_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
