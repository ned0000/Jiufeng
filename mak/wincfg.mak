#
#  @file wincfg.mak
#
#  @brief The configuration file for Windows platform.
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

JIUFENG_64BIT = yes

# Set variable depending on 64bit version or 32bit version.
!if "$(JIUFENG_64BIT)" == "yes"
CPU = X64
!else
CPU = X86
!endif

# Utility for nmake.
CC = cl
CXX = cl
LINK = link
RC = rc
CVTRES = cvtres
IMPLIB = lib
CP = copy

# Directory for binary and library files.
JIUTAI_DIR = $(TOPDIR)\jiutai
BUILD_DIR = $(TOPDIR)\build
BIN_DIR = $(BUILD_DIR)
LIB_DIR = $(BUILD_DIR)
CONFIG_DIR = $(BUILD_DIR)/config

# For including header files.
INCLUDES = /I$(JIUTAI_DIR) /I.\

# C flags.
CFLAGS = /DWINDOWS $(EXTRA_DEFS) /Zp8

# Set C flags depending on 64bit version or 32bit version.
!if "$(JIUFENG_64BIT)" == "yes"
CFLAGS = $(CFLAGS) /DJIUFENG_64BIT /DWIN64
!else
CFLAGS = $(CFLAGS) /DWIN32 /D_WIN32
!endif

# Link flags.
LDFLAGS = /NOLOGO

# DLL flags.
DLLFLAGS = /MACHINE:$(CPU) /LIBPATH:$(LIB_DIR) /INCREMENTAL:NO /NOLOGO /DLL

# Debug flags.
!if "$(DEBUG_JIUFENG)" == "yes"
CFLAGS = $(CFLAGS) /Zi /MTd
LDFLAGS = $(LDFLAGS) /DEBUG
DLLFLAGS = $(DLLFLAGS) /DEBUG
!else
CFLAGS = $(CFLAGS) /MT
LDFLAGS = $(LDFLAGS) /RELEASE
DLLFLAGS = $(DLLFLAGS) /RELEASE
!endif

# C++ flags.
CXXFLAGS = $(CFLAGS)

# System library.
SYSLIBS =

# For rc tools.
RFLAGS =

# For cvtres tools.
CVFLAGS = /machine:$(CPU)

#---------------------------------------------------------------------------------------------------


