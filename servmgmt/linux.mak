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

JIUTAI_SRCS = jf_mem.c sharedmemory.c process.c attask.c xtime.c

EXTRA_LIBS = -ljf_logger -lolfiles -lxml2

EXTRA_INC_DIR = -I/usr/include/libxml2

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

