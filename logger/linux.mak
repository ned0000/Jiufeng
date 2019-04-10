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

SONAME = jf_logger

SOURCES = common.c logger.c errcode.c

JIUTAI_SRCS = jf_hex.c

EXTRA_INC_DIR = 

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

