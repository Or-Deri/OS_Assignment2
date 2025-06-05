CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I../WareHouse --coverage

TARGETS = atom_supplier drinks_bar molecule_requester

all: $(TARGETS)

atom_supplier: atom_supplier.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

drinks_bar: drinks_bar.cpp ../WareHouse/WareHouse.cpp
	$(CXX) $(CXXFLAGS) -o $@ drinks_bar.cpp ../WareHouse/WareHouse.cpp

molecule_requester: molecule_requester.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.gcno *.gcda *.gcov

.PHONY: all clean
