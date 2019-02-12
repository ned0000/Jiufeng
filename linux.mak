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

SUBDIRS = jiutai logger stringparse files ifmgmt jiukun crypto encode uuid \
    persistency archive xmlparser httpparser network webclient test

include $(TOPDIR)/mak/lnxsubdirs.mak

#-----------------------------------------------------------------------------


