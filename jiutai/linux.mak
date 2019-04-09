#
#  @file linux.mak
#
#  @brief The makefile for Jiutai
#
#  @author Min Zhang
#
#  @note
#  

#-----------------------------------------------------------------------------

SOURCES = hexstr.c jf_process.c jf_thread.c xtime.c  \
    jf_stack.c jf_queue.c jf_linklist.c  \
    jf_dlinklist.c jf_hashtree.c jf_mem.c jf_mutex.c  \
    jf_rwlock.c jf_sem.c jf_array.c hash.c jf_menu.c crc32c.c  \
    sharedmemory.c jf_dynlib.c jf_hsm.c jf_host.c respool.c randnum.c  \
    jf_attask.c jf_sqlite.c

EXTRA_CFLAGS = -D_GNU_SOURCE

ifeq ("$(DEBUG_JIUFENG)", "yes")
EXTRA_CFLAGS += -DDEBUG_RADIXTREE -DDEBUG_PRIOTREE -DDEBUG_WAITQUEUE \
    -DDEBUG_WORKQUEUE -DDEBUG_AVLTREE
endif

EXTRA_INC_DIR = 

include $(TOPDIR)/mak/lnxobj.mak

#-----------------------------------------------------------------------------


