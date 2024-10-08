//
// Created by Ersi Bësi on 6.10.24.
//

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

        // TODO, think of way to make it possible for
        // TODO, Diffusion coefficient is also needed, learn how to adjust it
        // also quadratic or cubic interactions should be possible to, need to make it possible
        // perhaps best solution is to have a parser
        // vector<float> interactionCoefficients;  // represents how these species react to presence of other ones
};

#endif
