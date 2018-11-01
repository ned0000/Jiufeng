/**
 *  @file
 *
 *  @brief The configuration file for Linux platform 
 *
 *  @author Min Zhang
 *
 *  @note
 */

#-----------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxdef.mak

JIUTAI_DIR = $(TOPDIR)/jiutai
BUILD_DIR = $(TOPDIR)/build
BIN_DIR = $(BUILD_DIR)/bin
LIB_DIR = $(BUILD_DIR)/lib

INCLUDES = -I$(JIUTAI_DIR) -I./

LDFLAGS = -Wl,-rpath $(LIB_DIR)

SYSLIBS = -lpthread

# Debug flags
ifeq ("$(DEBUG_JIUFENG)", "yes")
CFLAGS += -g -DDEBUG
else
CFLAGS += -DNDEBUG
endif

#-----------------------------------------------------------------------------
