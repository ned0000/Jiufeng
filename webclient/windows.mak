#
#  @file windows.mak
#
#  @brief the makefile for webclient library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

DLLNAME = olwebclient
RESOURCE = webclient

SOURCES = webclient.c
JIUTAI_SRCS = $(JIUTAI_DIR)\bases.c $(JIUTAI_DIR)\jf_mem.c \
             $(JIUTAI_DIR)\jf_mutex.c $(JIUTAI_DIR)\xtime.c

EXTRA_LIBS = ws2_32.lib $(LIB_DIR)\ollogger.lib $(LIB_DIR)\olifmgmt.lib \
             $(LIB_DIR)\olhttpparser.lib $(LIB_DIR)\olnetwork.lib \
             $(LIB_DIR)\olstringparse.lib

EXTRA_DEFS = -DJIUFENG_WEBCLIENT_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------



