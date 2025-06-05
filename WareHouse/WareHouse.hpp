#ifndef WAREHOUSE_HPP
#define WAREHOUSE_HPP
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <mutex>

const unsigned long long MAX_ATOM_SIZE = 1000000000000000000;


// PART 2-
/*
WareHouse can supply the following molecules - 

// Water - H2O
// CarbonDioxide - CO2
// GLOCOUS - C6H12O6
// ETHNOL - C2H6O
*/

class Warehouse {
private:
    std::unordered_map<std::string, unsigned long long> atoms;
    mutable std::mutex mtx;

    std::vector<std::string> mol_available = {"WATER", "CARBON DIOXIDE", "ALCOHOL", "GLUCOSE"};

    std::vector<std::string> atoms_available = {"CARBON", "OXYGEN", "HYDROGEN"};

    std::vector<std::string> bar_drinks_available = {"SOFT DRINK", "VODKA", "CHAMPAGNE"};


public:
    Warehouse();

    bool add_atom(const std::string& type, unsigned int amount);

    bool build_molecules(const std::string molecule_type);

    void print_state() const;

    bool is_valid_atom(const std::string& type) const;

    bool is_valid_molecule(const std::string& mol) const;

    int build_drink_amount(const std::string& drink_type) const;


    unsigned long long get_CARBON() const;

    unsigned long long get_OXYGEN() const;

    unsigned long long get_HYDROGEN() const;

    bool set_atoms(const std::string &type, unsigned long long amount);
};

#endif
