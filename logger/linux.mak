#
#  @file linux.mak
#
#  @brief The makefile for logger library.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SONAME = jf_logger

SOURCES = common.c log2stdout.c log2systemlog.c log2tty.c log2file.c log2server.c \
    logger.c errcode.c

JIUTAI_SRCS = jf_hex.c jf_time.c jf_mem.c

EXTRA_INC_DIR = 

ifeq ("$(DEBUG_JIUFENG)", "yes")
#    EXTRA_CFLAGS += -DDEBUG_LOGGER
endif

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------

