#
#  @file linux.mak
#
#  @brief the makefile for logger library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SONAME = ollogger

SOURCES = common.c logger.c errcode.c

JIUTAI_SRCS = hexstr.c

EXTRA_INC_DIR = 

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

