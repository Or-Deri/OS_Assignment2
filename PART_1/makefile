CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fprofile-arcs -ftest-coverage
INCLUDE = -I../WareHouse

TARGET1 = atom_warehouse
TARGET2 = atom_supplier


all: $(TARGET1) $(TARGET2)

$(TARGET1): atom_warehouse.cpp ../WareHouse/WareHouse.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

$(TARGET2): atom_supplier.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET1) $(TARGET2) *.gcno *.gcov *.gcda ;

.PHONY: all run clean
