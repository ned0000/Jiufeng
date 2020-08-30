#
#  @file configmgr/daemon/windows.mak
#
#  @brief The makefile for config manager daemon.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

EXE = jf_configmgr.exe

CONFIG_FILES = configmgr.setting jiufeng.conf

SOURCES = ..\common\configmgrcommon.c configmgrsetting.c configpersistency.c \
    configtree.c configmgr.c main.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_option.c $(JIUTAI_DIR)\jf_process.c $(JIUTAI_DIR)\jf_ptree.c

EXTRA_INC_DIR = /I..\common

EXTRA_LIBS = psapi.lib jf_logger.lib jf_string.lib jf_jiukun.lib jf_xmlparser.lib \
    jf_persistency.lib jf_ifmgmt.lib jf_network.lib

!include "$(TOPDIR)\mak\winexe.mak"

#---------------------------------------------------------------------------------------------------


