#
#  @file linux.mak
#
#  @brief the makefile for UUID library
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SONAME = jf_uuid

SOURCES = uuid.c output.c

EXTRA_LIBS = -ljf_cghash -lolifmgmt -lolprng -ljf_string -ljf_files \
    -ljf_cghash

JIUTAI_SRCS = jf_time.c

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

