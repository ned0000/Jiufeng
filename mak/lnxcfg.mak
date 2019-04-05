#
#  @file
#
#  @brief The configuration file for Linux platform 
#
#  @author Min Zhang
#
#  @note
#

#-----------------------------------------------------------------------------

# Utility for make
CC = gcc
CXX = g++
LD = ld
AS = as
AR = ar
RANLIB = ranlib
CP = cp

# C flags
CFLAGS = -Wall -DLINUX -D_REENTRANT -fPIC
# for 64bit version
#CFLAGS += -DJIUFENG_64BIT
# Debug flags
ifeq ("$(DEBUG_JIUFENG)", "yes")
    CFLAGS += -g -DDEBUG
else
    CFLAGS += -DNDEBUG
endif

# C++ flags
CXXFLAGS = $(CFLAGS)

# Link flags
LDFLAGS = -Wl,-rpath $(LIB_DIR)

# Directory for make
JIUTAI_DIR = $(TOPDIR)/jiutai
BUILD_DIR = $(TOPDIR)/build
BIN_DIR = $(BUILD_DIR)/bin
LIB_DIR = $(BUILD_DIR)/lib

# Directory for header files
INCLUDES = -I$(JIUTAI_DIR) -I./

# System library
SYSLIBS = -lpthread

#-----------------------------------------------------------------------------
