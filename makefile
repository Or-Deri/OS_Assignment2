PARTS = PART_1 PART_2 PART_3 PART_4 PART_5 PART_6

# Default: build all parts (release)
all:
	@for part in $(PARTS); do \
		echo "Building $$part..."; \
		$(MAKE) -C $$part; \
	done

# Clean all parts
clean:
	@for part in $(PARTS); do \
		echo "Cleaning $$part..."; \
		$(MAKE) -C $$part clean; \
	done

# Build with gcov coverage
coverage:
	@for part in $(PARTS); do \
		echo "Building $$part with coverage..."; \
		$(MAKE) -C $$part coverage; \
	done

# Run gcov in all parts (assumes each subdir makefile supports 'gcov' target)
gcov:
	@for part in $(PARTS); do \
		echo "Running gcov in $$part..."; \
		$(MAKE) -C $$part gcov; \
	done

.PHONY: all clean coverage gcov
