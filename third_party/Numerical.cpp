#include "Numerical.h"
#include <vector>

vector<vector<int>> generateDispersionMatrix(int n);
vector<int> matrixVectorMultiplication(vector<int> vec, vector<vector<int>> matrix);
vector<int> computePopulation1DimDispersion(vector<int> populationVec, int boardWidth);
vector<vector<int>> computeSinglePopulationDispersion(vector<vector<int>> population);


vector<vector<vector<int>>> computePopulationsDispersion(vector<vector<vector<int>>> populations, vector<float> dispersionCoefficients) {
    int populationsCount = populations[0][0].size();

    // transform row-col-pop to pop-row-col
    vector<vector<vector<int>>> populationsTransformed = vector(populationsCount, vector(populations.size(), vector(populations[0].size(), 0)));
    for (int pop = 0; pop < populationsCount; pop++) {
        for (int row = 0; row < populations.size(); row++) {
            for (int col = 0; col < populations[0].size(); col++) {
                populationsTransformed[pop][row][col] = populations[row][col][pop];
            }
        }
    }

    for (int pop = 0; pop < populationsCount; pop++) {
        populationsTransformed[pop] = computeSinglePopulationDispersion(populationsTransformed[pop]);
    }

    // transform pop-row-col to row-col-pop
    vector<vector<vector<int>>> newPopulations = vector(populations.size(), vector(populations[0].size(), vector(populationsCount, 0)));
    for (int pop = 0; pop < populationsTransformed.size(); pop++) {
        for (int row = 0; row < populationsTransformed[0].size(); row++) {
            for (int col = 0; col < populationsTransformed[0][0].size(); col++) {
                newPopulations[row][col][pop] = populationsTransformed[pop][row][col];
            }
        }
    }

    return newPopulations;
}

vector<vector<int>> computeSinglePopulationDispersion(vector<vector<int>> population) {
    vector<vector<int>> newPopulationRows = vector(population.size(), vector(population[0].size(), 0));
    for (int row = 0; row < population.size(); row++) {
        newPopulationRows[row] = computePopulation1DimDispersion(population[row], population[0].size());
    }

    vector<vector<int>> newPopulationCols = vector(population[0].size(), vector(population.size(), 0));
    for (int col = 0; col < population[0].size(); col++) {
        vector<int> populationCol = vector(population.size(), 0);
        for (int row = 0; row < population.size(); row++) {
            populationCol[row] = population[row][col];
        }
        newPopulationCols[col] = computePopulation1DimDispersion(populationCol, population.size());
    }

    vector<vector<int>> newPopulation = vector(population.size(), vector(population[0].size(), 0));
    for (int row = 0; row < population.size(); row++) {
        for (int col = 0; col < population.size(); col++) {
            newPopulation[row][col] = newPopulationRows[row][col] + newPopulationCols[col][row];
        }
    }
    return newPopulation;
}

vector<int> computePopulation1DimDispersion(vector<int> populationVec, int dimLength) {
    if (dimLength == 1) {
        return populationVec;
    }
    auto dispersionMatrix = generateDispersionMatrix(dimLength);
    return matrixVectorMultiplication(populationVec, dispersionMatrix);
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

    for (int row = 0; row < matrix.size(); row++) {
        int sum = 0;
        for (int col = 0; col < matrix.size(); col++) {
            sum += vec[col] * matrix[row][col];
        }
        resultVector[row] = sum;
    }
    return resultVector;
}
