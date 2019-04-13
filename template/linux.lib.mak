#
#  @file
#
#  @brief The template makefile for Linux
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = oltemplate

# Source files
SOURCES = template.c

# Jiutai source files
JIUTAI_SRCS = jf_hex.c

# For code complile
EXTRA_INC_DIR = -I../kinc
EXTRA_CFLAGS = -DENT

# For library build 
EXTRA_LDFLAGS = 
EXTRA_OBJECTS =
EXTRA_LIB_DIR = 
EXTRA_LIBS = -ljf_logger

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

