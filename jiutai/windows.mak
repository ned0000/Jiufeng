#
#  @file jiutai/windows.mak
#
#  @brief The Makefile for Jiutai.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SOURCES = jf_option.c jf_hex.c jf_process.c jf_thread.c jf_time.c jf_date.c \
    jf_stack.c jf_queue.c jf_linklist.c jf_dlinklist.c jf_hashtree.c jf_mem.c jf_mutex.c \
    jf_rwlock.c jf_sem.c jf_array.c jf_hashtable.c jf_menu.c jf_crc.c  jf_ptree.c \
    jf_sharedmemory.c jf_dynlib.c jf_hsm.c jf_host.c jf_respool.c jf_rand.c jf_user.c \
    jf_attask.c

!if "$(DEBUG_JIUFENG)" == "yes"
EXTRA_CFLAGS = $(EXTRA_CFLAGS) /DDEBUG_PTREE
!endif

EXTRA_INC_DIR = 

!include "$(TOPDIR)\mak\winobj.mak"

#---------------------------------------------------------------------------------------------------
