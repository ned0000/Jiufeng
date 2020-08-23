#
#  @file archive/windows.mak
#
#  @brief The main Makefile for archive library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_archive
RESOURCE = archive

SOURCES = archive.c arfile.c create.c extract.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_linklist.c

EXTRA_LIBS = jf_logger.lib jf_files.lib jf_jiukun.lib jf_string.lib

EXTRA_INC_DIR =

EXTRA_DEFS = /DJIUFENG_ARCHIVE_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------
