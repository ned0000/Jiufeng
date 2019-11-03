#
#  @file linux.mak
#
#  @brief The Makefile for test
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak

PROGRAMS = xmalloc-test hashtree-test listhead-test                \
    listarray-test logger-test process-test                        \
    hashtable-test mutex-test rwlock-test sem-test date-test       \
    time-test stringparse-test bitarray-test conffile-test         \
    menu-test crc-test dynlib-test ifmgmt-test                     \
    sharedmemory-test-consumer sharedmemory-test-worker            \
    files-test hsm-test host-test respool-test                     \
    bitop-test jiukun-test cghash-test cgmac-test jf_genuuid       \
    encrypt-test prng-test encode-test xmlparser-test              \
    rand-test persistency-test archive-test                        \
    httpparser-test network-test network-test-server               \
    network-test-client network-test-client-chain                  \
    matrix-test webclient-test jf_service sqlite-test              \
    hex-test utimer-test

SOURCES = xmalloc-test.c hashtree-test.c listhead-test.c               \
    listarray-test.c logger-test.c process-test.c                      \
    hashtable-test.c mutex-test.c rwlock-test.c sem-test.c date-test.c \
    time-test.c stringparse-test.c bitarray-test.c conffile-test.c     \
    menu-test.c crc-test.c dynlib-test.c ifmgmt-test.c                 \
    sharedmemory-test-consumer.c sharedmemory-test-worker.c            \
    files-test.c hsm-test.c host-test.c respool-test.c                 \
    bitop-test.c jiukun-test.c cghash-test.c cgmac-test.c genuuid.c    \
    encrypt-test.c prng-test.c encode-test.c xmlparser-test.c          \
    rand-test.c persistency-test.c archive-test.c                      \
    httpparser-test.c network-test.c network-test-server.c             \
    network-test-client.c network-test-client-chain.c                  \
    matrix-test.c webclient-test.c servmgmt-test.c sqlite-test.c       \
    hex-test.c utimer-test.c

include $(TOPDIR)/mak/lnxobjdef.mak

FULL_PROGRAMS = $(foreach i, $(PROGRAMS), $(BIN_DIR)/$i)

EXTRA_CFLAGS = -D_GNU_SOURCE

EXTRA_INC_DIR =

all: $(FULL_PROGRAMS)

$(BIN_DIR)/time-test: time-test.o $(JIUTAI_DIR)/jf_time.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_string

$(BIN_DIR)/date-test: date-test.o $(JIUTAI_DIR)/jf_date.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_string

$(BIN_DIR)/hashtree-test: hashtree-test.o $(JIUTAI_DIR)/jf_hashtree.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_jiukun

$(BIN_DIR)/listhead-test: listhead-test.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_jiukun

$(BIN_DIR)/listarray-test: listarray-test.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_jiukun

$(BIN_DIR)/logger-test: logger-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/process-test: process-test.o $(JIUTAI_DIR)/jf_process.o $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/xmalloc-test: xmalloc-test.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS)

$(BIN_DIR)/sem-test: sem-test.o $(JIUTAI_DIR)/jf_sem.o $(JIUTAI_DIR)/jf_process.o \
       $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/mutex-test: mutex-test.o $(JIUTAI_DIR)/jf_mutex.o $(JIUTAI_DIR)/jf_process.o \
       $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/rwlock-test: rwlock-test.o $(JIUTAI_DIR)/jf_rwlock.o $(JIUTAI_DIR)/jf_process.o \
       $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/hashtable-test: hashtable-test.o $(JIUTAI_DIR)/jf_hashtable.o $(JIUTAI_DIR)/jf_process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_jiukun

$(BIN_DIR)/stringparse-test: stringparse-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_string -ljf_logger \
       -ljf_jiukun

$(BIN_DIR)/hex-test: hex-test.o $(JIUTAI_DIR)/jf_hex.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/bitarray-test: bitarray-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_string

$(BIN_DIR)/conffile-test: conffile-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_files \
       -ljf_jiukun

$(BIN_DIR)/menu-test: menu-test.o $(JIUTAI_DIR)/jf_menu.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_string -ljf_jiukun \
       -ljf_logger

$(BIN_DIR)/crc-test: crc-test.o $(JIUTAI_DIR)/jf_crc.o $(JIUTAI_DIR)/jf_hex.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/dynlib-test: dynlib-test.o $(JIUTAI_DIR)/jf_dynlib.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ldl -ljf_logger -ljf_jiukun

$(BIN_DIR)/sharedmemory-test-worker: sharedmemory-test-worker.o $(JIUTAI_DIR)/jf_sharedmemory.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_jiukun

$(BIN_DIR)/sharedmemory-test-consumer: sharedmemory-test-consumer.o $(JIUTAI_DIR)/jf_sharedmemory.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_jiukun

$(BIN_DIR)/ifmgmt-test: ifmgmt-test.o $(JIUTAI_DIR)/jf_process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_ifmgmt -ljf_string \
       -ljf_logger

$(BIN_DIR)/files-test: files-test.o $(JIUTAI_DIR)/jf_process.o $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_files \
       -ljf_string

$(BIN_DIR)/hsm-test: hsm-test.o $(JIUTAI_DIR)/jf_hsm.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/host-test: host-test.o $(JIUTAI_DIR)/jf_host.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_ifmgmt -ljf_string \
       -ljf_logger

$(BIN_DIR)/respool-test: respool-test.o $(JIUTAI_DIR)/jf_respool.o $(JIUTAI_DIR)/jf_mutex.o \
       $(JIUTAI_DIR)/jf_array.o $(JIUTAI_DIR)/jf_process.o $(JIUTAI_DIR)/jf_sem.o \
       $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_jiukun

$(BIN_DIR)/bitop-test: bitop-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) 

$(BIN_DIR)/jiukun-test: jiukun-test.o $(JIUTAI_DIR)/jf_process.o $(JIUTAI_DIR)/jf_mutex.o \
       $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_jiukun

$(BIN_DIR)/cghash-test: cghash-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_cghash -ljf_logger \
       -ljf_string

$(BIN_DIR)/cgmac-test: cgmac-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_cgmac -ljf_logger \
       -ljf_string

$(BIN_DIR)/encrypt-test: encrypt-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_encrypt -ljf_logger \
       -ljf_string -lssl -lcrypto -ljf_jiukun

$(BIN_DIR)/prng-test: prng-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_prng -ljf_logger \
       -ljf_cghash -ljf_jiukun

$(BIN_DIR)/encode-test: encode-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_encode -ljf_logger \
       -ljf_files -ljf_jiukun

$(BIN_DIR)/jf_genuuid: genuuid.o $(JIUTAI_DIR)/jf_time.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -ljf_uuid -ljf_prng -ljf_string -ljf_jiukun

$(BIN_DIR)/rand-test: rand-test.o $(JIUTAI_DIR)/jf_rand.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/persistency-test: persistency-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -lsqlite3 \
       -ljf_persistency -lsqlite3 -ljf_jiukun

$(BIN_DIR)/archive-test: archive-test.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_archive \
       -ljf_jiukun -ljf_string

$(BIN_DIR)/xmlparser-test: xmlparser-test.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -lm -ljf_xmlparser \
       -ljf_logger -ljf_jiukun

$(BIN_DIR)/httpparser-test: httpparser-test.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -lm -ljf_httpparser \
       -ljf_logger -ljf_jiukun

$(BIN_DIR)/network-test: network-test.o $(JIUTAI_DIR)/jf_process.o $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_network -ljf_string \
       -ljf_logger -ljf_ifmgmt -ljf_jiukun

$(BIN_DIR)/network-test-server: network-test-server.o $(JIUTAI_DIR)/jf_process.o \
       $(JIUTAI_DIR)/jf_thread.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_network -ljf_logger \
       -ljf_string -ljf_ifmgmt -ljf_jiukun

$(BIN_DIR)/network-test-client: network-test-client.o $(JIUTAI_DIR)/jf_process.o \
       $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_network -ljf_logger \
       -ljf_ifmgmt -ljf_jiukun

$(BIN_DIR)/network-test-client-chain: network-test-client-chain.o $(JIUTAI_DIR)/jf_process.o \
       $(JIUTAI_DIR)/jf_thread.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_network -ljf_logger \
       -ljf_ifmgmt -ljf_jiukun

$(BIN_DIR)/webclient-test: webclient-test.o $(JIUTAI_DIR)/jf_process.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_httpparser \
       -ljf_network -ljf_webclient -ljf_logger -ljf_files -ljf_ifmgmt -ljf_jiukun

$(BIN_DIR)/jf_service: servmgmt-test.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -ljf_serv \
       -ljf_jiukun

$(BIN_DIR)/matrix-test: matrix-test.o $(JIUTAI_DIR)/jf_option.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_matrix -ljf_logger \
       -ljf_jiukun

$(BIN_DIR)/sqlite-test: sqlite-test.o $(JIUTAI_DIR)/jf_sqlite.o $(JIUTAI_DIR)/jf_rand.o \
       $(JIUTAI_DIR)/jf_time.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_logger -lsqlite3 \
       -ljf_jiukun

$(BIN_DIR)/utimer-test: utimer-test.o $(JIUTAI_DIR)/jf_process.o $(JIUTAI_DIR)/jf_thread.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ -o $@ $(SYSLIBS) -ljf_network -ljf_logger \
       -ljf_ifmgmt -ljf_jiukun

include $(TOPDIR)/mak/lnxobjbld.mak

clean:
	rm -f $(FULL_PROGRAMS) $(OBJECTS)

#---------------------------------------------------------------------------------------------------


