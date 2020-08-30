#
#  @file persistency/linux.mak
#
#  @brief The makefile for persistency library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_persistency

SOURCES = persistencycommon.c sqlitepersistency.c filepersistency.c persistency.c

JIUTAI_SRCS = jf_time.c jf_rand.c jf_mutex.c jf_sqlite.c jf_hashtree.c

EXTRA_INC_DIR = 

EXTRA_LIBS = -ljf_jiukun -ljf_logger -ljf_files -ljf_string -lsqlite3

SYSLIBS = -lsqlite3

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------
