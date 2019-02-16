#
#  @file linux.mak
#
#  @brief the makefile for service manangement library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

SONAME = olservmgmt

SOURCES = servmgmt.c servmgmtsetting.c

JIUTAI_SRCS = xmalloc.c sharedmemory.c process.c attask.c xtime.c

EXTRA_LIBS = -lollogger -lolfiles -lxml2

EXTRA_INC_DIR = -I/usr/include/libxml2

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

