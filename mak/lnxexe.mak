#
#  @file
#
#  @brief The Makefile for building executable file on Linux platform
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak
include $(TOPDIR)/mak/lnxobjdef.mak

PROGRAM = $(BIN_DIR)/$(EXE)

TMP_JIUTAI_OBJS = $(foreach i,$(JIUTAI_SRCS),$(JIUTAI_DIR)/$i)

JIUTAI_OBJS = $(TMP_JIUTAI_OBJS:.c=.o)

all: $(PROGRAM)
	@for i in $(CONFIG_FILES); do \
        $(CP) -af $$i $(CONFIG_DIR); \
    done

$(PROGRAM) : $(OBJECTS) $(JIUTAI_OBJS)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $(OBJECTS) $(JIUTAI_OBJS) \
      $(EXTRA_OBJECTS) -o $(PROGRAM) $(SYSLIBS) $(EXTRA_LIB_DIR) $(EXTRA_LIBS)

include $(TOPDIR)/mak/lnxobjbld.mak

clean:
	rm -f $(PROGRAM) $(OBJECTS)

#---------------------------------------------------------------------------------------------------


