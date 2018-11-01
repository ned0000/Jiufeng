/**
 *  @file
 *
 *  @brief The definition file for Linux platform 
 *
 *  @author Min Zhang
 *
 *  @note
 */

#-----------------------------------------------------------------------------

CC = gcc
CXX = g++
LD = ld
AS = as
AR = ar
RANLIB = ranlib
CP = cp
INCLUDES = -I./
CFLAGS = -Wall -DLINUX -D_REENTRANT -fPIC
CXXFLAGS = $(CFLAGS)
LDFLAGS = 

# for 64bit version
CFLAGS += -DJIUTAI_64BIT

#-----------------------------------------------------------------------------

