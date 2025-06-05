# all the sub files to compile
SUBDIRS = PART_1 PART_2 PART_3 PART_4 PART_5 PART_6

# targets
all: $(SUBDIRS:%=build_%)

# build the targets
build_%:
	$(MAKE) -C $* all

run: $(SUBDIRS:%=run_%)

run_%:
	$(MAKE) -C $* run

gcov: $(SUBDIRS:%=gcov_%)

gcov_%:
	$(MAKE) -C $* gcov

clean: $(SUBDIRS:%=clean_%)

clean_%:
	$(MAKE) -C $* clean

.PHONY: all run gcov clean \
        $(SUBDIRS:%=build_%) \
        $(SUBDIRS:%=run_%) \
        $(SUBDIRS:%=gcov_%) \
        $(SUBDIRS:%=clean_%)
