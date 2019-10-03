#
#  @file linux.mak
#
#  @brief the makefile for service manangement library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_servmgmt

CONFIG_FILES = servmgmt.setting

SOURCES = servmgmt.c servmgmtsetting.c

JIUTAI_SRCS = jf_mem.c jf_sharedmemory.c jf_process.c jf_attask.c jf_time.c

EXTRA_LIBS = -ljf_logger -ljf_files -lxml2

EXTRA_INC_DIR = -I/usr/include/libxml2

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

