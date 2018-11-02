#
#  @file
#
#  @brief The makefile for sub-directory on Windows platform 
#
#  @author Min Zhang
#
#  @note
#

#-----------------------------------------------------------------------------

!include $(TOPDIR)\mak\wincfg.mak

all:
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@for %%d in ($(SUBDIRS)) do \
	@    (cd %%d & nmake /NOLOGO -f windows.mak all & cd ..)

clean:
	@for %%d in ($(SUBDIRS)) do \
	@    (cd %%d & nmake -f windows.mak clean & cd ..)
	@if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)

#-----------------------------------------------------------------------------


