/**
 *  @file
 *
 *  @brief The makefile for sub-directory on Linux platform 
 *
 *  @author Min Zhang
 *
 *  @note
 */

#-----------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak

.PHONY: $(SUBDIRS)

.builddirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(LIB_DIR)
	@touch .builddirs

all: .builddirs subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ -f linux.mak 2>&1 | tee $@.make.out
	@echo "---------------------------------------------"
	@echo "$@ warnings found: `grep -c ': warning:' $@.make.out`"
	@echo "$@ errors found: `grep -c 'make\[.*\]*\*\*\*.*Error' $@.make.out`"
	@echo "---------------------------------------------"

clean:
	@for i in $(SUBDIRS); do \
		($(MAKE) -C $$i -f linux.mak clean; rm -f $$i.make.out) \
	done
	@rm -rf $(BUILD_DIR)
	@rm -f .builddirs

#-----------------------------------------------------------------------------

