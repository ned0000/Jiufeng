#
#  @file linux.mak
#
#  @brief The Makefile for cryptographic message authentication code
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

SOURCES = hmac.c
SONAME = olcgmac

JIUTAI_SRCS = xmalloc.c

EXTRA_LIBS = -lolcghash

EXTRA_INC_DIR = -I../cghash

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

