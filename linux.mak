#
#  @file linux.mak
#
#  @brief The Makefile for Jiufeng
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

TOPDIR  := $(shell /bin/pwd)

export TOPDIR

SUBDIRS = jiutai logger stringparse test

include $(TOPDIR)/mak/lnxsubdirs.mak

#-----------------------------------------------------------------------------


