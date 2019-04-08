#
#  @file linux.mak
#
#  @brief the makefile for persistency library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

SONAME = olpersistency

SOURCES = persistency.c sqlitepersistency.c

JIUTAI_SRCS = xtime.c randnum.c syncmutex.c jf_mem.c jtsqlite.c

EXTRA_INC_DIR = 

SYSLIBS = -lsqlite3

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

