#include "Numerical.h"
#include <vector>

vector<vector<int>> generateDispersionMatrix(int n);


vector<vector<vector<int>>> computePopulationsDispersion(vector<vector<vector<int>>> populations, vector<float> dispersionCoefficients, int boardWidth, int boardHeight) {
    auto dispersionMatrix = vector(boardHeight, vector(boardWidth, 0));
    auto newPopulations = vector(populations.size(), NULL);

    for (auto& population : populations) {
    }
    return populations;
}

vector<vector<int>> computeSinglePopulationDispersion(vector<vector<int>> population, float dispersionCoefficient) {
    return population;
}

vector<int> computePopulationRowDispersion(vector<int> population, int boardWidth) {
    vector<int> newPopulation = vector(population);
    if (boardWidth == 1) {
        return population;
    }

    auto dispersionMatrix = generateDispersionMatrix(boardWidth);
    for (int i = 0; i < boardWidth; i++) {

    }
}

vector<vector<int>> generateDispersionMatrix(int n) {
    auto dispersionMatrix = vector(n, vector(n, 0));
    for (int i = 0; i < n; i++) {
        if (i == 0) {
            dispersionMatrix[i][i] = -1;
            dispersionMatrix[i][i + 1] = 1;
        } else if (i == n - 1) {
            dispersionMatrix[i][i - 1] = 1;
            dispersionMatrix[i][i] = -1;
        } else {
            dispersionMatrix[i][i - 1] = 1;
            dispersionMatrix[i][i] = -2;
            dispersionMatrix[i][i + 1] = 1;
        }
    }
    return dispersionMatrix;
}

vector<int> matrixVectorMultiplication(vector<int> vec, vector<vector<int>> matrix) {
    vector<int> resultVector = vector(vec.size(), 0);

}
