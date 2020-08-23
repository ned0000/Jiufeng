#
#  @file test/windows.mak
#
#  @brief The Makefile for test files on Windows platform.
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

!include $(TOPDIR)\mak\wincfg.mak

PROGRAMS = $(BIN_DIR)\mem-test.exe $(BIN_DIR)\option-test.exe $(BIN_DIR)\hashtree-test.exe        \
    $(BIN_DIR)\listhead-test.exe $(BIN_DIR)\hlisthead-test.exe $(BIN_DIR)\listarray-test.exe      \
    $(BIN_DIR)\logger-test.exe $(BIN_DIR)\process-test.exe $(BIN_DIR)\thread-test.exe             \
    $(BIN_DIR)\hashtable-test.exe $(BIN_DIR)\mutex-test.exe $(BIN_DIR)\rwlock-test.exe            \
    $(BIN_DIR)\sem-test.exe $(BIN_DIR)\date-test.exe $(BIN_DIR)\time-test.exe                     \
    $(BIN_DIR)\string-test.exe $(BIN_DIR)\bitarray-test.exe $(BIN_DIR)\conffile-test.exe          \
    $(BIN_DIR)\menu-test.exe $(BIN_DIR)\crc-test.exe $(BIN_DIR)\dynlib-test.exe                   \
    $(BIN_DIR)\ifmgmt-test.exe $(BIN_DIR)\sharedmemory-test-consumer.exe                          \
    $(BIN_DIR)\sharedmemory-test-worker.exe $(BIN_DIR)\files-test.exe $(BIN_DIR)\hsm-test.exe     \
    $(BIN_DIR)\host-test.exe $(BIN_DIR)\respool-test.exe $(BIN_DIR)\bitop-test.exe                \
    $(BIN_DIR)\ptree-test.exe $(BIN_DIR)\jiukun-test.exe $(BIN_DIR)\cghash-test.exe               \
    $(BIN_DIR)\cgmac-test.exe $(BIN_DIR)\encrypt-test.exe $(BIN_DIR)\dlinklist-test.exe           \
    $(BIN_DIR)\prng-test.exe $(BIN_DIR)\encode-test.exe $(BIN_DIR)\xmlparser-test.exe             \
    $(BIN_DIR)\rand-test.exe $(BIN_DIR)\persistency-test.exe $(BIN_DIR)\archive-test.exe          \
    $(BIN_DIR)\user-test.exe $(BIN_DIR)\httpparser-test.exe $(BIN_DIR)\network-test.exe           \
    $(BIN_DIR)\linklist-test.exe $(BIN_DIR)\network-test-server.exe                               \
    $(BIN_DIR)\network-test-client.exe $(BIN_DIR)\network-test-client-chain.exe                   \
    $(BIN_DIR)\matrix-test.exe $(BIN_DIR)\webclient-test.exe $(BIN_DIR)\hex-test.exe              \
    $(BIN_DIR)\utimer-test.exe

SOURCES = mem-test.c option-test.c hashtree-test.c listhead-test.c hlisthead-test.c           \
    listarray-test.c logger-test.c process-test.c thread-test.c hashtable-test.c mutex-test.c \
    rwlock-test.c sem-test.c date-test.c time-test.c string-test.c                            \
    bitarray-test.c conffile-test.c menu-test.c crc-test.c dynlib-test.c                      \
    ifmgmt-test.c sharedmemory-test-consumer.c sharedmemory-test-worker.c                     \
    files-test.c hsm-test.c host-test.c respool-test.c bitop-test.c ptree-test.c              \
    jiukun-test.c cghash-test.c cgmac-test.c encrypt-test.c dlinklist-test.c                  \
    prng-test.c encode-test.c xmlparser-test.c rand-test.c persistency-test.c                 \
    archive-test.c user-test.c httpparser-test.c network-test.c linklist-test.c               \
    network-test-server.c network-test-client.c network-test-client-chain.c                   \
    matrix-test.c webclient-test.c hex-test.c utimer-test.c

!include $(TOPDIR)\mak\winobjdef.mak

FULL_PROGRAMS = $(PROGRAMS)

EXTRA_CFLAGS =

EXTRA_INC_DIR =

all: $(FULL_PROGRAMS)

$(BIN_DIR)\mem-test.exe: mem-test.obj $(JIUTAI_DIR)\jf_mem.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS)

$(BIN_DIR)\option-test.exe: option-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib

$(BIN_DIR)\hashtree-test.exe: hashtree-test.obj $(JIUTAI_DIR)\jf_hashtree.obj \
       $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib

$(BIN_DIR)\listhead-test.exe: listhead-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\hlisthead-test.exe: hlisthead-test.obj $(JIUTAI_DIR)\jf_option.obj \
       $(JIUTAI_DIR)\jf_hashtable.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib

$(BIN_DIR)\listarray-test.exe: listarray-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib

$(BIN_DIR)\logger-test.exe: logger-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_thread.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\process-test.exe: process-test.obj $(JIUTAI_DIR)\jf_process.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       ws2_32.lib Psapi.lib

$(BIN_DIR)\thread-test.exe: thread-test.obj $(JIUTAI_DIR)\jf_thread.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\hashtable-test.exe: hashtable-test.obj $(JIUTAI_DIR)\jf_hashtable.obj \
       $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_process.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\mutex-test.exe: mutex-test.obj $(JIUTAI_DIR)\jf_mutex.obj $(JIUTAI_DIR)\jf_thread.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\rwlock-test.exe: rwlock-test.obj $(JIUTAI_DIR)\jf_rwlock.obj $(JIUTAI_DIR)\jf_thread.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\sem-test.exe: sem-test.obj $(JIUTAI_DIR)\jf_sem.obj $(JIUTAI_DIR)\jf_thread.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\date-test.exe: date-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_date.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\time-test.exe: time-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_time.obj \
       $(JIUTAI_DIR)\jf_date.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\string-test.exe: string-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_string.lib

$(BIN_DIR)\bitarray-test.exe: bitarray-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_string.lib

$(BIN_DIR)\conffile-test.exe: conffile-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_files.lib

$(BIN_DIR)\menu-test.exe: menu-test.obj $(JIUTAI_DIR)\jf_process.obj $(JIUTAI_DIR)\jf_menu.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_string.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\crc-test.exe: crc-test.obj $(JIUTAI_DIR)\jf_crc.obj $(JIUTAI_DIR)\jf_hex.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       ws2_32.lib

$(BIN_DIR)\dynlib-test.exe: dynlib-test.obj $(JIUTAI_DIR)\jf_dynlib.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib

$(BIN_DIR)\ifmgmt-test.exe: ifmgmt-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_process.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_ifmgmt.lib jf_string.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\sharedmemory-test-consumer.exe: sharedmemory-test-consumer.obj $(JIUTAI_DIR)\jf_sharedmemory.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_uuid.lib

$(BIN_DIR)\sharedmemory-test-worker.exe: sharedmemory-test-worker.obj $(JIUTAI_DIR)\jf_sharedmemory.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_uuid.lib

$(BIN_DIR)\files-test.exe: files-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_thread.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_files.lib jf_string.lib

$(BIN_DIR)\hsm-test.exe: hsm-test.obj $(JIUTAI_DIR)\jf_hsm.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib

$(BIN_DIR)\host-test.exe: host-test.obj $(JIUTAI_DIR)\jf_host.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_string.lib jf_ifmgmt.lib Iphlpapi.lib User32.lib Advapi32.lib

$(BIN_DIR)\respool-test.exe: respool-test.obj $(JIUTAI_DIR)\jf_respool.obj $(JIUTAI_DIR)\jf_mutex.obj \
       $(JIUTAI_DIR)\jf_array.obj $(JIUTAI_DIR)\jf_process.obj $(JIUTAI_DIR)\jf_thread.obj \
       $(JIUTAI_DIR)\jf_sem.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\bitop-test.exe: bitop-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS)

$(BIN_DIR)\ptree-test.exe: ptree-test.obj $(JIUTAI_DIR)\jf_ptree.obj $(JIUTAI_DIR)\jf_option.obj \
       $(JIUTAI_DIR)\jf_linklist.obj $(JIUTAI_DIR)\jf_hashtree.obj $(JIUTAI_DIR)\jf_stack.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_string.lib jf_jiukun.lib

$(BIN_DIR)\jiukun-test.exe: jiukun-test.obj $(JIUTAI_DIR)\jf_process.obj $(JIUTAI_DIR)\jf_mutex.obj \
       $(JIUTAI_DIR)\jf_thread.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\cghash-test.exe: cghash-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_hex.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_string.lib jf_cghash.lib

$(BIN_DIR)\cgmac-test.exe: cgmac-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_hex.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_string.lib jf_cgmac.lib

$(BIN_DIR)\encrypt-test.exe: encrypt-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_string.lib jf_encrypt.lib jf_jiukun.lib

$(BIN_DIR)\dlinklist-test.exe: dlinklist-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_dlinklist.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib

$(BIN_DIR)\prng-test.exe: prng-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_hex.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_cghash.lib jf_prng.lib

$(BIN_DIR)\encode-test.exe: encode-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_encode.lib jf_files.lib

$(BIN_DIR)\xmlparser-test.exe: xmlparser-test.obj $(JIUTAI_DIR)\jf_option.obj \
       $(JIUTAI_DIR)\jf_ptree.obj $(JIUTAI_DIR)\jf_linklist.obj $(JIUTAI_DIR)\jf_hashtree.obj \
       $(JIUTAI_DIR)\jf_stack.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_xmlparser.lib jf_string.lib

$(BIN_DIR)\rand-test.exe: rand-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_rand.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\persistency-test.exe: persistency-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_persistency.lib

$(BIN_DIR)\archive-test.exe: archive-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_linklist.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_archive.lib

$(BIN_DIR)\user-test.exe: user-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_user.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\httpparser-test.exe: httpparser-test.obj $(JIUTAI_DIR)\jf_option.obj \
       $(JIUTAI_DIR)\jf_hex.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_httpparser.lib

$(BIN_DIR)\network-test.exe: network-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_thread.obj \
       $(JIUTAI_DIR)\jf_process.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_network.lib jf_string.lib jf_ifmgmt.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\linklist-test.exe: linklist-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_linklist.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib

$(BIN_DIR)\network-test-server.exe: network-test-server.obj $(JIUTAI_DIR)\jf_option.obj \
       $(JIUTAI_DIR)\jf_thread.obj $(JIUTAI_DIR)\jf_process.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_network.lib jf_string.lib jf_ifmgmt.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\network-test-client.exe: network-test-client.obj $(JIUTAI_DIR)\jf_option.obj \
       $(JIUTAI_DIR)\jf_thread.obj $(JIUTAI_DIR)\jf_process.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_network.lib jf_string.lib jf_ifmgmt.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\network-test-client-chain.exe: network-test-client-chain.obj $(JIUTAI_DIR)\jf_option.obj \
       $(JIUTAI_DIR)\jf_thread.obj $(JIUTAI_DIR)\jf_process.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_network.lib jf_string.lib jf_ifmgmt.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\matrix-test.exe: matrix-test.obj $(JIUTAI_DIR)\jf_option.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_matrix.lib

$(BIN_DIR)\webclient-test.exe: webclient-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_process.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_network.lib jf_string.lib jf_ifmgmt.lib jf_webclient.lib jf_files.lib \
       jf_httpparser.lib ws2_32.lib Psapi.lib

$(BIN_DIR)\hex-test.exe: hex-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_hex.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib

$(BIN_DIR)\utimer-test.exe: utimer-test.obj $(JIUTAI_DIR)\jf_option.obj $(JIUTAI_DIR)\jf_thread.obj \
       $(JIUTAI_DIR)\jf_process.obj
	@$(LINK) $(LDFLAGS) $(EXTRA_LDFLAGS) /LIBPATH:$(LIB_DIR) /OUT:$@ $** $(SYSLIBS) jf_logger.lib \
       jf_jiukun.lib jf_network.lib jf_ifmgmt.lib ws2_32.lib Psapi.lib

!include $(TOPDIR)\mak\winobjbld.mak

clean:
	@for %%x in ($(OBJECTS)) do \
	@   if exist %%x del %%x

#---------------------------------------------------------------------------------------------------
