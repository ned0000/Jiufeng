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

#---------------------------------------------------------------------------------------------------

SOURCES = hmac.c
SONAME = jf_cgmac

JIUTAI_SRCS = 

EXTRA_LIBS = -ljf_cghash

EXTRA_INC_DIR = -I../cghash

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

