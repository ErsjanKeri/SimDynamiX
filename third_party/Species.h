//
// Created by Ersi Bësi on 6.10.24.
//

#ifndef SPECIES_H
#define SPECIES_H
#include <string>
#include <iostream>
using namespace std;


class Block {
    public:
        bool free = true;
    };

class Species : Block {
    // are there many per block or one
    public:
        Species(const string& species_name, const u_int species_color)
                : Block(), name(species_name), color(species_color) {
            free = false;  // Set free to false when a species occupies the block
        }
        string name = "Species";
        u_int color = 0xffffffff;
};

#endif
