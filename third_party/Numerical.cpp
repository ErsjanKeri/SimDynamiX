#include "Numerical.h"
#include <vector>

vector<vector<int>> computePopulationsDispersion(vector<vector<int>> populations, vector<float> dispersionCoefficients, int boardWidth, int boardHeight) {
    auto dispersionMatrix = vector(boardHeight, vector(boardWidth, 0));
    auto newPopulations = vector(populations.size(), NULL);

    for (auto& population : populations) {
    }
    return populations;
}

vector<int> computeSinglePopulationDispersion(vector<int> population, float dispersionCoefficient, vector<vector<int>> dispersionMatrix) {
    return population;
}

