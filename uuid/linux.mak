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

SONAME = oluuid

SOURCES = uuid.c output.c

EXTRA_LIBS = -lolcghash -lolifmgmt -lolprng -lolstringparse -lolfiles \
    -lolcghash

JIUTAI_SRCS = xtime.c

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

