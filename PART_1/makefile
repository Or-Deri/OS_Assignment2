CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
GCOVFLAGS = --coverage

SOURCES = atom_supplier.cpp molecule_requester.cpp drinks_bar.cpp 
TARGETS = $(SOURCES:.cpp=)

all: $(TARGETS)

coverage: CXXFLAGS += $(GCOVFLAGS)
coverage: clean all

gcov:
	@for file in $(SOURCES); do \
		[ -f $${file%.cpp}.gcda ] && gcov $$file; \
	done

clean:
	rm -f $(TARGETS) *.o *.gcda *.gcno *.gcov

.PHONY: all coverage gcov clean
