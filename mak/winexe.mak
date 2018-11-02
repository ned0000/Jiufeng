#
#  @file
#
#  @brief The Makefile for building executable file on Windows platform
#
#  @author Min Zhang
#
#  @note
#

#-----------------------------------------------------------------------------

!include $(TOPDIR)\mak\wincfg.mak
!include $(TOPDIR)\mak\winobjdef.mak
!include $(TOPDIR)\mak\winobjbld.mak

PROGRAM = $(BIN_DIR)\$(EXE)

JIUTAI_OBJS = $(JIUTAI_SRCS:.c=.obj)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(JIUTAI_OBJS)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(link) $(LDFLAGS) $(EXTRA_LDFLAGS) /libpath:$(LIB_DIR) \
	    /out:$@ $** $(SYSLIBS) $(EXTRA_LIBS)

clean:
	@if exist $(PROGRAM) del $(PROGRAM)
    @if exist *.pdb del *.pdb
	@for %%x in ($(OBJECTS)) do \
	@    if exist %%x del %%x

#-----------------------------------------------------------------------------

