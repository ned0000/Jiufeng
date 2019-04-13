#
#  @file
#
#  @brief The Makefile for building dynamic library on Windows platform
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
	@$(rc) $(RFLAGS) $(INCLUDES) -r $(RESOURCE).rc

$(RESOURCE).rbj: $(RESOURCE).res
	@cvtres $(CVFLAGS) -o $(RESOURCE).rbj $(RESOURCE).res

$(DLL_LIBFILE): $(OBJECTS)
	@$(implib) /OUT:$(DLL_LIBFILE) $(OBJECTS) $(JIUTAI_OBJS)

$(DLLFILE): $(OBJECTS) $(DLL_LIBFILE) $(JIUTAI_OBJS) $(RESOURCE).rbj
	@if not exist $(LIB_DIR) mkdir $(LIB_DIR)
	$(link) $(DLLFLAGS) /implib:$(DLL_LIBFILE) $(EXTRA_DLLFLAGS) \
	    /out:$@ $(OBJECTS) $(JIUTAI_OBJS) $(RESOURCE).rbj $(SYSLIBS) \
	    $(EXTRA_LIBS)

clean:
	@for %%x in (obj dll lib exp exe pdb ilk res rbj) do \
	@    if exist *.%%x del *.%%x
	@for %%x in (dll pdb exp lib) do \
	@    if exist $(LIB_DIR)\$(DLLNAME).%%x del $(LIB_DIR)\$(DLLNAME).%%x

#---------------------------------------------------------------------------------------------------

