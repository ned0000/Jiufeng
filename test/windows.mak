#
#  @file test/windows.mak
#
#  @brief The Makefile for test files on Windows platform.
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

!include $(TOPDIR)\mak\wincfg.mak

PROGRAMS = $(BIN_DIR)\mem-test.exe $(BIN_DIR)\logger-test.exe $(BIN_DIR)\jiukun-test.exe \
    $(BIN_DIR)\listhead-test.exe

SOURCES = mem-test.c logger-test.c jiukun-test.c listhead-test.c

!include $(TOPDIR)\mak\winobjdef.mak

FULL_PROGRAMS = $(PROGRAMS)

EXTRA_CFLAGS =

EXTRA_INC_DIR =

all: $(FULL_PROGRAMS)

$(BIN_DIR)\mem-test.exe: mem-test.obj $(JIUTAI_DIR)\jf_mem.obj
	$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /libpath:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS)

$(BIN_DIR)\logger-test.exe: logger-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_thread.obj
	$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /libpath:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\jiukun-test.exe: jiukun-test.obj $(JIUTAI_DIR)\jf_process.obj $(JIUTAI_DIR)\jf_mutex.obj \
       $(JIUTAI_DIR)\jf_thread.obj $(JIUTAI_DIR)\jf_option.obj
	$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /libpath:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\listhead-test.exe: listhead-test.obj $(JIUTAI_DIR)\jf_option.obj
	$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /libpath:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

!include $(TOPDIR)\mak\winobjbld.mak

clean:
	@for %%x in ($(OBJECTS)) do \
	@   if exist %%x del %%x

#---------------------------------------------------------------------------------------------------
