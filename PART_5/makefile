CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g --coverage
INCLUDE = -I../WareHouse

TARGETS = drinks_bar atom_supplier molecule_requester

all: $(TARGETS)

drinks_bar: drinks_bar.cpp ../WareHouse/WareHouse.cpp ../WareHouse/WareHouse.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ drinks_bar.cpp ../WareHouse/WareHouse.cpp

atom_supplier: atom_supplier.cpp
	$(CXX) $(CXXFLAGS) -o $@ atom_supplier.cpp

molecule_requester: molecule_requester.cpp
	$(CXX) $(CXXFLAGS) -o $@ molecule_requester.cpp

clean:
	rm -f $(TARGETS) *.gcno *.gcda *.gcov

.PHONY: all clean
