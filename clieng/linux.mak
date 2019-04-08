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

SONAME = olclieng

SOURCES = clieng.c engio.c cmdparser.c cmdhistory.c 

JIUTAI_SRCS = xmalloc.c hash.c bases.c syncmutex.c process.c hexstr.c

EXTRA_LIBS = -ljf_logger -lolstringparse

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

