#
#  @file Makefile
#
#  @brief The makefile for network library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

SONAME = olnetwork

SOURCES = internalsocket.c socket.c socketpair.c \
    chain.c utimer.c asocket.c assocket.c acsocket.c \
    adgram.c resolve.c network.c

JIUTAI_SRCS = jf_mem.c syncmutex.c xtime.c

EXTRA_LIBS = -ljf_logger

ifeq ("$(DEBUG_JIUFENG)", "yes")
    EXTRA_CFLAGS = # -DDEBUG_CHAIN -DDEBUG_UTIMER
endif

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------

