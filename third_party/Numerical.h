#ifndef NUMERICAL_H
#define NUMERICAL_H

#include <vector>

void computeChangedPopulation(std::vector<std::vector<std::vector<double>>> & board, std::vector<std::vector<double>> & coefficients);
void computePopulationsDispersion(std::vector<std::vector<std::vector<double>>> & populations, std::vector<double> dispersionCoefficients);
void prepareCalculations();

#endif // NUMERICAL_H