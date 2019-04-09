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

#-----------------------------------------------------------------------------

SONAME = jf_clieng

SOURCES = clieng.c engio.c cmdparser.c cmdhistory.c 

JIUTAI_SRCS = jf_mem.c jf_hashtable.c jf_mutex.c jf_process.c hexstr.c

EXTRA_LIBS = -ljf_logger -ljf_string

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

