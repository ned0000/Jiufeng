#
#  @file Makefile
#
#  @brief The Makefile for dongyuan
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_configmgr

CONFIG_FILES = configmgr.setting jiufeng.conf

SOURCES = ../common/configmgrcommon.c configmgrsetting.c configpersistency.c configtree.c \
    configmgr.c main.c

JIUTAI_SRCS = jf_process.c jf_mutex.c jf_thread.c jf_option.c jf_ptree.c

EXTRA_LIBS = -ljf_string -ljf_files -ljf_logger -ljf_ifmgmt -ljf_network -ljf_jiukun -ljf_xmlparser

EXTRA_INC_DIR = -I../common

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------


