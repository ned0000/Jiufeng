#
#  @file
#
#  @brief The Makefile for building dynamic library on Linux platform
#
#  @author Min Zhang
#
#  @note
#

#-----------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak
include $(TOPDIR)/mak/lnxobjdef.mak
include $(TOPDIR)/mak/lnxobjbld.mak

SHAREOBJ = $(LIB_DIR)/lib$(SONAME).so

TMP_JIUTAI_OBJS = $(foreach i,$(JIUTAI_SRCS),$(JIUTAI_DIR)/$i)

JIUTAI_OBJS = $(TMP_JIUTAI_OBJS:.c=.o)

all: $(SHAREOBJ)

firstsource = $(firstword $(SOURCES))

iscpp = $(findstring .cpp, $(firstsource))

ifneq  "$(iscpp)" ""
$(SHAREOBJ): $(OBJECTS) $(JIUTAI_OBJS)
	$(CXX) -shared $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ $(EXTRA_OBJECTS)\
      -o $@ $(SYSLIBS) $(EXTRA_LIB_DIR) $(EXTRA_LIBS) -fPIC
else
$(SHAREOBJ): $(OBJECTS) $(JIUTAI_OBJS)
	$(CC) -shared $(LDFLAGS) $(EXTRA_LDFLAGS) -L$(LIB_DIR) $^ $(EXTRA_OBJECTS) \
      -o $@ $(SYSLIBS) $(EXTRA_LIB_DIR) $(EXTRA_LIBS) -fPIC
endif

clean:
	rm -f $(SHAREOBJ) $(OBJECTS)

#-----------------------------------------------------------------------------

