CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
INCLUDE = -I../WareHouse

TARGETS = atom_warehouse atom_supplier

all: $(TARGETS)

atom_warehouse: atom_warehouse.cpp ../WareHouse/WareHouse.cpp ../WareHouse/WareHouse.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ atom_warehouse.cpp ../WareHouse/WareHouse.cpp

atom_supplier: atom_supplier.cpp
	$(CXX) $(CXXFLAGS) -o $@ atom_supplier.cpp

clean:
	rm -f $(TARGETS)

.PHONY: all clean
