#
#  @file linux.mak
#
#  @brief The makefile for Jiufeng.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

TOPDIR := $(shell /bin/pwd)

export TOPDIR

SUBDIRS = jiutai logger jiukun string files ifmgmt crypto encode uuid  \
    persistency archive xmlparser httpparser network webclient clieng  \
    matrix servmgmt logserver configmgr dispatcher cli utility test

include $(TOPDIR)/mak/lnxsubdirs.mak

#---------------------------------------------------------------------------------------------------


