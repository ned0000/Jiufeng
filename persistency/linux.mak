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

JIUTAI_SRCS = jf_time.c randnum.c jf_mutex.c jf_mem.c jf_sqlite.c

EXTRA_INC_DIR = 

SYSLIBS = -lsqlite3

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

