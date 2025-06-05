#include "WareHouse.hpp"
#include <iostream>

Warehouse::Warehouse()
{
    atoms["HYDROGEN"] = 0;
    atoms["OXYGEN"] = 0;
    atoms["CARBON"] = 0;
}

bool Warehouse::add_atom(const std::string &type, unsigned int amount)
{
    std::lock_guard<std::mutex> lock(mtx);

    if (!is_valid_atom(type))
        return false;

    if (amount <= 0)
    {
        return false;
    }

    if (atoms[type] + amount <= MAX_ATOM_SIZE)
    {
        atoms[type] += amount;
        return true;
    }
    return false;
}

bool Warehouse::is_valid_atom(const std::string &type) const
{
    for (const auto &name : atoms_available)
    {
        if (name == type)
        {
            return true;
        }
    }
    return false;
}

bool Warehouse::is_valid_molecule(const std::string &mol) const
{
    for (const auto &name : mol_available)
    {
        if (name == mol)
        {
            return true;
        }
    }
    return false;
}

void Warehouse::print_state() const
{
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Inventory updated:\n";
    for (const auto &[k, v] : atoms)
    {
        std::cout << "  " << k << ": " << v << "\n";
    }
}

int Warehouse::build_drink_amount(const std::string &drink_type) const
{
    std::lock_guard<std::mutex> lock(mtx);

    int water = std::min((int)(atoms.at("HYDROGEN") / 2), (int)atoms.at("OXYGEN"));

    int carbon_dioxide = std::min((int)atoms.at("CARBON"), (int)(atoms.at("OXYGEN") / 2));

    int alcohol = std::min((int)(atoms.at("CARBON") / 2), (int)(atoms.at("HYDROGEN") / 6));

    alcohol = std::min(alcohol, (int)atoms.at("OXYGEN"));

    int glucose = std::min((int)(atoms.at("CARBON") / 6), (int)(atoms.at("HYDROGEN") / 12));

    glucose = std::min(glucose, (int)(atoms.at("OXYGEN") / 6));

    if (drink_type == "CHAMPAGNE")
    {
        int min_count = water;
        if (carbon_dioxide < min_count)
            min_count = carbon_dioxide;
        if (alcohol < min_count)
            min_count = alcohol;
        return min_count;
    }
    else if (drink_type == "VODKA")
    {
        int min_count = water;
        if (alcohol < min_count)
            min_count = alcohol;
        if (glucose < min_count)
            min_count = glucose;
        return min_count;
    }
    else if (drink_type == "SOFT DRINK")
    {
        int min_count = water;
        if (carbon_dioxide < min_count)
            min_count = carbon_dioxide;
        if (glucose < min_count)
            min_count = glucose;
        return min_count;
    }
    return 0;
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

    if (molecule_type == "WATER")
    {
        need_HYDROGEN = 2;
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
    else
    {
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
    atoms["HYDROGEN"] -= need_HYDROGEN;
    return true;
}

unsigned long long Warehouse::get_CARBON() const
{
    return atoms.at("CARBON");
}

unsigned long long Warehouse::get_OXYGEN() const
{
    return atoms.at("OXYGEN");
}

unsigned long long Warehouse::get_HYDROGEN() const
{
    return atoms.at("HYDROGEN");
}

bool Warehouse::set_atoms(const std::string& atom, unsigned long long amount)
{
    if (!is_valid_atom(atom))
    {
        return false;
    }
    atoms[atom] = amount;
    return true;
}
