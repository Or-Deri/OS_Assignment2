#compiler
CXX = g++

#flags
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I../WareHouse --coverage

#targets
all: atom_supplier molecule_supplier molecule_requester

atom_supplier: atom_supplier.cpp
	$(CXX) $(CXXFLAGS) -o atom_supplier atom_supplier.cpp

molecule_supplier: molecule_supplier.cpp ../WareHouse/WareHouse.cpp ../WareHouse/WareHouse.hpp
	$(CXX) $(CXXFLAGS) -o molecule_supplier  molecule_supplier.cpp ../WareHouse/WareHouse.cpp

molecule_requester: molecule_requester.cpp
	$(CXX) $(CXXFLAGS) -o molecule_requester molecule_requester.cpp

clean:
	rm -f atom_supplier molecule_supplier molecule_requester