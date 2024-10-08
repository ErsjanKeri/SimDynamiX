#include "Numerical.h"
#include <vector>

vector<vector<int>> generateDispersionMatrix(int n);
vector<int> matrixVectorMultiplication(vector<int> vec, vector<vector<int>> matrix);
vector<int> computePopulation1DimDispersion(vector<int> populationVec);
vector<vector<int>> computeSinglePopulationDispersion(vector<vector<int>> population);


// computes the dispersed populations
// @param populations the populations in a row-col-pop format
// @param dispersionCoefficients the coefficients of dispersion for each population
// @return the dispersed populations
vector<vector<vector<int>>> computePopulationsDispersion(vector<vector<vector<int>>> populations, vector<float> dispersionCoefficients) {
    // transform row-col-pop to pop-row-col
    vector<vector<vector<int>>> populationsTransformed = vector(populations[0][0].size(), vector(populations.size(), vector(populations[0].size(), 0)));
    for (int pop = 0; pop < populations[0][0].size(); pop++) {
        for (int row = 0; row < populations.size(); row++) {
            for (int col = 0; col < populations[0].size(); col++) {
                populationsTransformed[pop][row][col] = populations[row][col][pop];
            }
        }
    }

    // compute dispersion for every population
    for (int pop = 0; pop < populations[0][0].size(); pop++) {
        populationsTransformed[pop] = computeSinglePopulationDispersion(populationsTransformed[pop]);
    }

    // transform pop-row-col to row-col-pop
    vector<vector<vector<int>>> newPopulations = vector(populations.size(), vector(populations[0].size(), vector(populations[0][0].size(), 0)));
    for (int pop = 0; pop < populationsTransformed.size(); pop++) {
        for (int row = 0; row < populationsTransformed[0].size(); row++) {
            for (int col = 0; col < populationsTransformed[0][0].size(); col++) {
                newPopulations[row][col][pop] = populationsTransformed[pop][row][col];
            }
        }
    }

    return newPopulations;
}

// computes the dispersion for a single population
// @param population the population in a row-col format
// @return the dispersed population
vector<vector<int>> computeSinglePopulationDispersion(vector<vector<int>> population) {
    // compute the 1D dispersion for every row
    // newPopulationCols is in a row-col format
    vector<vector<int>> newPopulationRows = vector(population.size(), vector(population[0].size(), 0));
    for (int row = 0; row < population.size(); row++) {
        newPopulationRows[row] = computePopulation1DimDispersion(population[row]);
    }

    // compute the 1D dispersion for every column
    // newPopulationCols is in a col-row format
    vector<vector<int>> newPopulationCols = vector(population[0].size(), vector(population.size(), 0));
    for (int col = 0; col < population[0].size(); col++) {
        vector<int> populationCol = vector(population.size(), 0);
        for (int row = 0; row < population.size(); row++) {
            populationCol[row] = population[row][col];
        }
        newPopulationCols[col] = computePopulation1DimDispersion(populationCol);
    }

    // combine the 1D dispersions to yield the 2D dispersion
    vector<vector<int>> newPopulation = vector(population.size(), vector(population[0].size(), 0));
    for (int row = 0; row < population.size(); row++) {
        for (int col = 0; col < population[0].size(); col++) {
            newPopulation[row][col] = newPopulationRows[row][col] + newPopulationCols[col][row];
        }
    }
    return newPopulation;
}

// computes the dispersion in 1D
// @param populationVec the vector representing the population in 1D
// @return the dispersed 1D population
vector<int> computePopulation1DimDispersion(vector<int> populationVec) {
    if (populationVec.size() == 1) {
        return populationVec;
    }
    auto dispersionMatrix = generateDispersionMatrix(populationVec.size());
    return matrixVectorMultiplication(populationVec, dispersionMatrix);
}

// generates the dispersion matrix to perform a 1D dispersion
// @param n the dimension of the matrix (the matrix will always be quadratic)
// @return the dispersion matrix
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

// performs a matrix-vector multiplication
// @param vec the vector in the multiplication
// @param matrix the matrix in the multiplication
// @return the result of the multiplication
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


// performs simple scalar-product
// @param animals the vector containing the population of animals at this cell
// @param coefficients the vector expressing the influence of other populations on our current
float scalarProduct(vector<int> & animals, vector<float> & coefficients) {
    float res = 0;
    for (int i = 0; i < animals.size(); i++) {
        res += static_cast<float>(animals[i])*coefficients[i];
    }
    return res;
}


// performs the population change on all cells
// @param board is the whole board with populations
// @param coefficients is how animal at index i is influenced from other population
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
