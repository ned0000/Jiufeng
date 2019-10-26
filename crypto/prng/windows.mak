#
#  @file windows.mak
#
#  @brief The makefile for pseudo random number generator library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

DLLNAME = jf_prng
RESOURCE = prng

SOURCES = ..\common\clrmem.c prng.c seed.c

JIUTAI_SRCS = $(JIUTAI_DIR)\jf_mutex.c $(JIUTAI_DIR)\jf_time.c

EXTRA_LIBS = jf_logger.lib jf_cghash.lib Netapi32.lib

EXTRA_INC_DIR = -I..\common

EXTRA_DEFS = -DJIUFENG_PRNG_DLL

!include "$(TOPDIR)\mak\winlib.mak"

#---------------------------------------------------------------------------------------------------


