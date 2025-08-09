#ifndef SPECIES_H
#define SPECIES_H
#include <string>
#include <iostream>
using namespace std;


class Species {
    // are there many per block or one
    public:
        Species(const string& species_name, const u_int species_color)
                : name(species_name), color(species_color) {
        }
        string name = "Species";
        u_int color = 0xffffffff;
};

#endif