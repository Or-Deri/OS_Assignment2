CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I../WareHouse

all: atom_supplier drinks_bar molecule_requester

atom_supplier: atom_supplier.cpp
	$(CXX) $(CXXFLAGS) -o atom_supplier atom_supplier.cpp

drinks_bar: drinks_bar.cpp ../WareHouse/WareHouse.cpp ../WareHouse/WareHouse.hpp
	$(CXX) $(CXXFLAGS) -o drinks_bar drinks_bar.cpp ../WareHouse/WareHouse.cpp

molecule_requester: molecule_requester.cpp
	$(CXX) $(CXXFLAGS) -o molecule_requester molecule_requester.cpp

clean:
	rm -f atom_supplier drinks_bar molecule_requester