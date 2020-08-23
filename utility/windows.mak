#
#  @file utility/windows.mak
#
#  @brief The Makefile for utility on Windows platform.
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

!include $(TOPDIR)\mak\wincfg.mak

PROGRAMS = $(BIN_DIR)\jf_errcode.exe $(BIN_DIR)\jf_genuuid.exe

SOURCES = errcode.c genuuid.c

!include $(TOPDIR)\mak\winobjdef.mak

FULL_PROGRAMS = $(PROGRAMS)

EXTRA_CFLAGS =

EXTRA_INC_DIR =

all: $(FULL_PROGRAMS)

$(BIN_DIR)\jf_errcode.exe: errcode.obj $(JIUTAI_DIR)\jf_option.obj
	$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_string.lib

$(BIN_DIR)\jf_genuuid.exe: genuuid.obj $(JIUTAI_DIR)\jf_time.obj $(JIUTAI_DIR)\jf_option.obj
	$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_uuid.lib jf_prng.lib jf_string.lib jf_jiukun.lib

!include $(TOPDIR)\mak\winobjbld.mak

clean:
	@for %%x in ($(OBJECTS)) do \
	@   if exist %%x del %%x

#---------------------------------------------------------------------------------------------------
