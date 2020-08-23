#
#  @file 
#
#  @brief The Makefile for building executable file.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the executable file
EXE = olexe

# Source files
SOURCES = template.c

# Jiutai source files
JIUTAI_SRCS = jf_hex.c

# For code complile
EXTRA_INC_DIR = -I../kinc
EXTRA_CFLAGS = -DENT

# For executable file build 
EXTRA_LDFLAGS =
EXTRA_OBJECTS =
EXTRA_LIB_DIR =
EXTRA_LIBS = -ljf_logger

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------
