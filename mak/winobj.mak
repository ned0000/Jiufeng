#
#  @file
# 
#  @brief The Makefile for building object file on Windows platform
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

!include $(TOPDIR)/mak/wincfg.mak
!include $(TOPDIR)/mak/winobjdef.mak

all: $(OBJECTS)

!include $(TOPDIR)/mak/winobjbld.mak

clean:
	@for %%x in (obj) do \
	@    if exist *.%%x del *.%%x

#---------------------------------------------------------------------------------------------------


