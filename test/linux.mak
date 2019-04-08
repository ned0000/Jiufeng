#
#  @file linux.mak
#
#  @brief The Makefile for test
#
#  @author Min Zhang
#
#  @note
#

#-----------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak

PROGRAMS = xmalloc-test bases-test logger-test process-test   \
    hash-test syncmutex-test syncrwlock-test syncsem-test     \
    xtime-test stringparse-test bitarray-test conffile-test   \
    menu-test crc32c-test dynlib-test ifmgmt-test             \
    sharedmemory-test-consumer sharedmemory-test-worker       \
    files-test hsm-test host-test respool-test            \
    bitop-test jiukun-test cghash-test cgmac-test genuuid     \
    encrypt-test prng-test encode-test xmlparser-test         \
    randnum-test persistency-test archive-test                \
    httpparser-test network-test network-test-server          \
    network-test-client network-test-client-chain             \
    matrix-test webclient-test olservmgmt sqlite-test       \
    hexstr-test

SOURCES = xmalloc-test.c bases-test.c logger-test.c process-test.c   \
    hash-test.c syncmutex-test.c syncrwlock-test.c syncsem-test.c    \
    xtime-test.c stringparse-test.c bitarray-test.c conffile-test.c  \
    menu-test.c crc32c-test.c dynlib-test.c ifmgmt-test.c            \
    sharedmemory-test-consumer.c sharedmemory-test-worker.c          \
    files-test.c hsm-test.c host-test.c respool-test.c           \
    bitop-test.c jiukun-test.c cghash-test.c cgmac-test.c genuuid.c  \
    encrypt-test.c prng-test.c encode-test.c xmlparser-test.c        \
    randnum-test.c persistency-test.c archive-test.c                 \
    httpparser-test.c network-test.c network-test-server.c           \
    network-test-client.c network-test-client-chain.c                \
    matrix-test.c webclient-test.c servmgmt-test.c sqlite-test.c   \
    hexstr-test.c

include $(TOPDIR)/mak/lnxobjdef.mak

FULL_PROGRAMS = $(foreach i, $(PROGRAMS), $(BIN_DIR)/$i)

EXTRA_CFLAGS = -D_GNU_SOURCE

EXTRA_INC_DIR =

all: $(FULL_PROGRAMS)

$(BIN_DIR)/xtime-test: xtime-test.o $(JIUTAI_DIR)/xtime.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolstringparse

$(BIN_DIR)/bases-test: bases-test.o $(JIUTAI_DIR)/bases.o \
       $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/logger-test: logger-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/process-test: process-test.o $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/xmalloc-test: xmalloc-test.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS)

$(BIN_DIR)/syncsem-test: syncsem-test.o $(JIUTAI_DIR)/syncsem.o \
       $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/syncmutex-test: syncmutex-test.o $(JIUTAI_DIR)/syncmutex.o \
       $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/syncrwlock-test: syncrwlock-test.o $(JIUTAI_DIR)/syncrwlock.o \
       $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/hash-test: hash-test.o $(JIUTAI_DIR)/hash.o $(JIUTAI_DIR)/jf_mem.o \
       $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/stringparse-test: stringparse-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolstringparse -ljf_logger

$(BIN_DIR)/hexstr-test: hexstr-test.o $(JIUTAI_DIR)/hexstr.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/bitarray-test: bitarray-test.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -lolstringparse

$(BIN_DIR)/conffile-test: conffile-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -lolfiles

$(BIN_DIR)/menu-test: menu-test.o $(JIUTAI_DIR)/jf_mem.o $(JIUTAI_DIR)/jf_menu.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolstringparse

$(BIN_DIR)/crc32c-test: crc32c-test.o $(JIUTAI_DIR)/crc32c.o \
       $(JIUTAI_DIR)/hexstr.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/dynlib-test: dynlib-test.o $(JIUTAI_DIR)/jf_dynlib.o \
       $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ldl -ljf_logger

$(BIN_DIR)/sharedmemory-test-worker: sharedmemory-test-worker.o \
       $(JIUTAI_DIR)/sharedmemory.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/sharedmemory-test-consumer: sharedmemory-test-consumer.o \
       $(JIUTAI_DIR)/sharedmemory.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/ifmgmt-test: ifmgmt-test.o $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolifmgmt -lolstringparse -ljf_logger

$(BIN_DIR)/files-test: files-test.o $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -lolfiles -lolstringparse

$(BIN_DIR)/hsm-test: hsm-test.o $(JIUTAI_DIR)/jf_hsm.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/host-test: host-test.o $(JIUTAI_DIR)/jf_host.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolifmgmt -lolstringparse -ljf_logger

$(BIN_DIR)/respool-test: respool-test.o $(JIUTAI_DIR)/respool.o \
       $(JIUTAI_DIR)/syncmutex.o $(JIUTAI_DIR)/jf_mem.o $(JIUTAI_DIR)/array.o \
       $(JIUTAI_DIR)/process.o $(JIUTAI_DIR)/syncsem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/bitop-test: bitop-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) 

$(BIN_DIR)/jiukun-test: jiukun-test.o $(JIUTAI_DIR)/process.o \
       $(JIUTAI_DIR)/syncmutex.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -loljiukun

$(BIN_DIR)/cghash-test: cghash-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolcghash -ljf_logger -lolstringparse

$(BIN_DIR)/cgmac-test: cgmac-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolcgmac -ljf_logger -lolstringparse

$(BIN_DIR)/encrypt-test: encrypt-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolencrypt -ljf_logger -lolstringparse -lssl -lcrypto

$(BIN_DIR)/prng-test: prng-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolprng -ljf_logger -lolcghash

$(BIN_DIR)/encode-test: encode-test.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolencode -ljf_logger -lolfiles

$(BIN_DIR)/genuuid: genuuid.o $(JIUTAI_DIR)/xtime.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -loluuid -lolprng

$(BIN_DIR)/randnum-test: randnum-test.o $(JIUTAI_DIR)/randnum.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger

$(BIN_DIR)/persistency-test: persistency-test.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -lolpersistency -lsqlite3

$(BIN_DIR)/archive-test: archive-test.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -lolarchive

$(BIN_DIR)/xmlparser-test: xmlparser-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lm -lolxmlparser -ljf_logger

$(BIN_DIR)/httpparser-test: httpparser-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lm -lolhttpparser -ljf_logger

$(BIN_DIR)/network-test: network-test.o $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolnetwork -lolstringparse -ljf_logger -lolifmgmt

$(BIN_DIR)/network-test-server: network-test-server.o \
       $(JIUTAI_DIR)/process.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolnetwork -ljf_logger -lolstringparse -lolifmgmt

$(BIN_DIR)/network-test-client: network-test-client.o $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolnetwork -ljf_logger -lolifmgmt

$(BIN_DIR)/network-test-client-chain: network-test-client-chain.o \
       $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolnetwork -ljf_logger -lolifmgmt

$(BIN_DIR)/webclient-test: webclient-test.o $(JIUTAI_DIR)/jf_mem.o \
       $(JIUTAI_DIR)/process.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolhttpparser -lolnetwork -lolwebclient -ljf_logger \
       -lolfiles -lolifmgmt

$(BIN_DIR)/olservmgmt: servmgmt-test.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -lolservmgmt

$(BIN_DIR)/matrix-test: matrix-test.o $(JIUTAI_DIR)/jf_mem.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lolmatrix -ljf_logger

$(BIN_DIR)/sqlite-test: sqlite-test.o $(JIUTAI_DIR)/jf_mem.o \
       $(JIUTAI_DIR)/jf_sqlite.o $(JIUTAI_DIR)/randnum.o $(JIUTAI_DIR)/xtime.o
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -ljf_logger -lsqlite3

include $(TOPDIR)/mak/lnxobjbld.mak

clean:
	rm -f $(FULL_PROGRAMS) $(OBJECTS)

#-----------------------------------------------------------------------------


