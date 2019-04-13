#
#  @file linux.mak
#
#  @brief The makefile for Jiutai
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SOURCES = jf_hex.c jf_process.c jf_thread.c jf_time.c jf_date.c  \
    jf_stack.c jf_queue.c jf_linklist.c jf_dlinklist.c jf_hashtree.c jf_mem.c jf_mutex.c  \
    jf_rwlock.c jf_sem.c jf_array.c jf_hashtable.c jf_menu.c jf_crc.c  \
    jf_sharedmemory.c jf_dynlib.c jf_hsm.c jf_host.c jf_respool.c jf_rand.c  \
    jf_attask.c jf_sqlite.c

EXTRA_CFLAGS = -D_GNU_SOURCE

ifeq ("$(DEBUG_JIUFENG)", "yes")
EXTRA_CFLAGS += -DDEBUG_RADIXTREE -DDEBUG_PRIOTREE -DDEBUG_WAITQUEUE \
    -DDEBUG_WORKQUEUE -DDEBUG_AVLTREE
endif

EXTRA_INC_DIR = 

include $(TOPDIR)/mak/lnxobj.mak

#---------------------------------------------------------------------------------------------------


