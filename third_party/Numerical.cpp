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


float scalarProduct(vector<int> & animals, vector<float> & coefficients) {
    float res = 0;
    for (int i = 0; i < animals.size(); i++) {
        res += static_cast<float>(animals[i])*coefficients[i];
    }
    return res;
}

void computeChangedPopulation(vector<vector<vector<int>>> & board, vector<vector<float>> & coefficients) {
    // dp/dt = coef * arr
    for (int y = 0; y < board.size(); y++) {
        for (int x = 0; x < board[0].size(); x++) {
            for (int k = 0; k < board[0][0].size(); k++) {
                board[y][x][k] += static_cast<int>(scalarProduct(board[y][x], coefficients[k]));
            }
        }
    }
}