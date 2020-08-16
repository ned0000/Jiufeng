#
#  @file lnxobj.mak
#
#  @brief The Makefile for building object file on Linux platform.
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak
include $(TOPDIR)/mak/lnxobjdef.mak

all: $(OBJECTS)

include $(TOPDIR)/mak/lnxobjbld.mak

clean:
	rm -f $(OBJECTS)

#---------------------------------------------------------------------------------------------------


