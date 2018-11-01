/**
 *  @file
 *
 *  @brief The configuration file for Windows platform 
 *
 *  @author Min Zhang
 *
 *  @note
 */

#-----------------------------------------------------------------------------

!include <ntwin32.mak>

CP = copy

JIUTAI_DIR = $(TOPDIR)\jiutai
BUILD_DIR = $(TOPDIR)\build
BIN_DIR = $(BUILD_DIR)
LIB_DIR = $(BUILD_DIR)

INCLUDES = -I$(JIUTAI_DIR) -I.\

CFLAGS = -DWIN32 -D_WIN32 -DWINDOWS $(EXTRA_DEFS) /Zp8

LDFLAGS = /nologo

DLLFLAGS = /SUBSYSTEM:windows /nologo /machine:$(CPU) \
           /LIBPATH:$(LIB_DIR) \
	   /INCREMENTAL:NO /NOLOGO -entry:_DllMainCRTStartup@12 -dll

# Debug flags
!if "$(DEBUG_JIUFENG)" == "yes"
CFLAGS = $(CFLAGS) /Zi /MTd
LDFLAGS = $(LDFLAGS) /DEBUG
DLLFLAGS = $(DLLFLAGS) /DEBUG
!else
CFLAGS = $(CFLAGS) /MT
LDFLAGS = $(LDFLAGS) /RELEASE
DLLFLAGS = $(DLLFLAGS) /RELEASE
!endif

CXXFLAGS = $(CFLAGS)

SYSLIBS = kernel32.lib advapi32.lib user32.lib

# for rc tools
RFLAGS =

# for cvtres tools 
CVFLAGS = -$(CPU)

#-----------------------------------------------------------------------------


