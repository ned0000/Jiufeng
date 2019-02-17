#
#  @file windows.mak
#
#  @brief The main makefile for cli engine
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

DLLNAME = olclieng
RESOURCE = clieng

SOURCES = clieng.c cmdhistory.c cmdparser.c engio.c

JIUTAI_SRCS = $(JIUTAI_DIR)\xmalloc.c $(JIUTAI_DIR)\syncmutex.c  \
    $(JIUTAI_DIR)\hash.c $(JIUTAI_DIR)\bases.c $(JIUTAI_DIR)\hexstr.c \
    $(JIUTAI_DIR)\process.c

EXTRA_LIBS = $(LIB_DIR)\ollogger.lib $(LIB_DIR)\olstringparse.lib \

EXTRA_DEFS = -DJIUFENG_CLIENG_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#-----------------------------------------------------------------------------


