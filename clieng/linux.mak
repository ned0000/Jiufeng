#
#  @file linux.mak
#
#  @brief the makefile for cli engine library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_clieng

SOURCES = clieng.c engio.c cmdparser.c cmdhistory.c 

JIUTAI_SRCS = jf_mem.c jf_hashtable.c jf_mutex.c jf_process.c jf_hex.c

EXTRA_LIBS = -ljf_logger -ljf_string -ljf_files -ljf_jiukun

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

