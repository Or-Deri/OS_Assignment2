#include "WareHouse.hpp"
#include <iostream>

Warehouse::Warehouse() {
    // first initialize the Warehouse atoms capacity to 0.
    atoms["HYDROGEN"] = 0;
    atoms["OXYGEN"] = 0;
    atoms["CARBON"] = 0;

    molecules["WATER"] = 0;
    molecules["CARBON DIOXIDE"] = 0;
    molecules["ALCOHOL"] = 0;
    molecules["GLUCOSE"] = 0;
}

bool Warehouse::add_atom(const std::string& type, unsigned int amount) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!is_valid_atom(type)) return false;

    if (atoms[type] + amount <= MAX_ATOM_SIZE) {
        atoms[type] += amount;
        return true;
    }
    return false;
}

bool Warehouse::is_valid_atom(const std::string& type) const {
    for (const auto& name : atoms_available) {
        if (name == type) {
            return true;
        }
    }
    return false;
}

bool Warehouse::is_valid_molecule(const std::string& mol) const {
    for (const auto& name : mol_available) {
        if (name == mol) {
            return true;
        }
    }
    return false;
}

bool Warehouse::deliver_molecules(const std::string& molecule_type, unsigned int amount) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!is_valid_molecule(molecule_type)) return false;

    if (molecules[molecule_type] < amount) return false;

    // deduct 1 molecule
    molecules[molecule_type] -= amount;
    return true;
}

void Warehouse::print_state() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Inventory updated:\n";
    for (const auto& [k, v] : atoms) {
        std::cout << "  " << k << ": " << v << "\n";
    }
}

bool Warehouse::is_valid_drink(const std::string& drink_type) const
{
    for (const auto& name : bar_drinks_available) {
        if (name == drink_type) {
            return true;
        }
    }
    return false;
}

int Warehouse::build_drink_amount(const std::string& drink_type) const {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (!is_valid_drink(drink_type)) return 0;
    
    int max_drinks = INT_MAX;
    
    if (drink_type == "SOFT DRINK") {
        // SOFT DRINK = WATER + CARBON DIOXIDE + GLUCOSE
        int water_available = molecules.at("WATER");
        int co2_available = molecules.at("CARBON DIOXIDE");
        int glucose_available = molecules.at("GLUCOSE");
        max_drinks = std::min({water_available, co2_available, glucose_available});
    }
    else if (drink_type == "VODKA") {
        // VODKA = WATER + ALCOHOL
        int water_available = molecules.at("WATER");
        int alcohol_available = molecules.at("ALCOHOL");
        max_drinks = std::min(water_available, alcohol_available);
    }
    else if (drink_type == "CHAMPAGNE") {
        // CHAMPAGNE = WATER + CARBON DIOXIDE + ALCOHOL + GLUCOSE
        int water_available = molecules.at("WATER");
        int co2_available = molecules.at("CARBON DIOXIDE");
        int alcohol_available = molecules.at("ALCOHOL");
        int glucose_available = molecules.at("GLUCOSE");
        max_drinks = std::min({water_available, co2_available, alcohol_available, glucose_available});
    }
    
    return max_drinks;
}

bool Warehouse::build_molecules(const std::string molecule_type) 
{
    std::lock_guard<std::mutex> lock(mtx);

    if (!is_valid_molecule(molecule_type)) 
    {
        return false;
    }

   /*
        WATER NEED 2 H + 1 C
        CARBON DIOXIDE NEECDS 1 C + 2 O
        ALCOHOL NEEDS 2 C + 6 H + 1 O
        GLUCOSE NEEDS 6 C + 12 H + 6 O
    */
    unsigned long long need_CARBON = 0;
    unsigned long long need_OXYGEN = 0;
    unsigned long long need_HYDROGEN = 0;

    if (molecule_type=="WATER") 
    {
        need_HYDROGEN =2;
        need_OXYGEN = 1;
    }
    else if (molecule_type == "CARBON DIOXIDE") 
    {
        need_CARBON = 1;
        need_OXYGEN = 2;
    }
    else if (molecule_type == "GLUCOSE") 
    {
        need_CARBON = 6;
        need_HYDROGEN = 12;
        need_OXYGEN = 6;
    } 
    else {
        need_OXYGEN = 1;
        need_HYDROGEN = 6;
        need_CARBON = 2;
    }

    // "CARBON", "OXYGEN", "HYDROGEN"
    if (atoms["CARBON"] < need_CARBON || atoms["OXYGEN"] < need_OXYGEN || atoms["HYDROGEN"] < need_HYDROGEN) 
    {
        return false;
    }

    atoms["CARBON"] -= need_CARBON;
    atoms["OXYGEN"] -= need_OXYGEN;
    atoms["HYDROGEN"] -=need_HYDROGEN;
    molecules[molecule_type] += 1;
    return true;
}

