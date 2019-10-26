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

#---------------------------------------------------------------------------------------------------

SONAME = jf_persistency

SOURCES = persistency.c sqlitepersistency.c

JIUTAI_SRCS = jf_time.c jf_rand.c jf_mutex.c jf_sqlite.c

EXTRA_INC_DIR = 

EXTRA_LIBS = -ljf_jiukun

SYSLIBS = -lsqlite3

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

