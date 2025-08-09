
#ifndef SPECIES_H
#define SPECIES_H
#include <string>
using namespace std;


class Species {
    // are there many per block or one
    public:
        string name;
        int color;
        Species(const string& species_name, const u_int species_color);
};

#endif
