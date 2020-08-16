#
#  @file winlib.mak
#
#  @brief The Makefile for building dynamic library on Windows platform.
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

!include $(TOPDIR)\mak\wincfg.mak
!include $(TOPDIR)\mak\winobjdef.mak
!include $(TOPDIR)\mak\winobjbld.mak

DLLFILE = $(LIB_DIR)\$(DLLNAME).dll
DLL_LIBFILE = $(LIB_DIR)\$(DLLNAME).lib

JIUTAI_OBJS = $(JIUTAI_SRCS:.c=.obj)

all: $(DLLFILE)

$(RESOURCE).res: $(RESOURCE).rc
	$(RC) $(RFLAGS) $(INCLUDES) -r $(RESOURCE).rc

$(RESOURCE).rbj: $(RESOURCE).res
	$(CVTRES) $(CVFLAGS) /OUT:$(RESOURCE).rbj $(RESOURCE).res

$(DLL_LIBFILE): $(OBJECTS) $(JIUTAI_OBJS)
	$(IMPLIB) /OUT:$(DLL_LIBFILE) $(OBJECTS) $(JIUTAI_OBJS)

#$(DLLFILE): $(DLL_LIBFILE) $(RESOURCE).rbj

$(DLLFILE): $(DLL_LIBFILE) $(OBJECTS) $(JIUTAI_OBJS) $(RESOURCE).rbj
	@if not exist $(LIB_DIR) mkdir $(LIB_DIR)
	$(LINK) $(DLLFLAGS) /IMPLIB:$(DLL_LIBFILE) $(EXTRA_DLLFLAGS) \
	    /OUT:$@ $(OBJECTS) $(JIUTAI_OBJS) $(RESOURCE).rbj $(SYSLIBS) $(EXTRA_LIBS)

clean:
	@for %%x in (obj dll lib exp exe pdb ilk res rbj) do \
	@    if exist *.%%x del *.%%x
	@for %%x in (dll pdb exp lib) do \
	@    if exist $(LIB_DIR)\$(DLLNAME).%%x del $(LIB_DIR)\$(DLLNAME).%%x

#---------------------------------------------------------------------------------------------------

