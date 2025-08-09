#include "Numerical.h"
#include <algorithm>
#include <vector>
#include "./settings.h"


vector<vector<double>> generateDispersionMatrix(int n);
vector<double> matrixVectorMultiplication(vector<double> vec, vector<vector<double>> matrix);
vector<double> computePopulation1DimDispersion(vector<double> populationVec);
vector<vector<double>> computeSinglePopulationDispersion(vector<vector<double>> population, double dispersionCoefficient);

// ADI helpers (Crank–Nicolson)
static void build_tridiagonal_neumann(int n, double r, std::vector<double>& a, std::vector<double>& b, std::vector<double>& c);
static void build_tridiagonal_dirichlet(int n, double r, std::vector<double>& a, std::vector<double>& b, std::vector<double>& c);
static void thomas_solve_inplace(std::vector<double>& a, std::vector<double>& b, std::vector<double>& c, std::vector<double>& d);
static inline double laplace_neumann_y(const std::vector<std::vector<double>>& u, int y, int x);
static inline double laplace_neumann_x(const std::vector<std::vector<double>>& u, int y, int x);
static std::vector<std::vector<double>> computeSinglePopulationDispersionADI(const std::vector<std::vector<double>>& population, double diffusionCoefficient);
static std::vector<std::vector<double>> computeSinglePopulationDispersionExplicitNeumann(const std::vector<std::vector<double>>& population);


// computes the dispersed populations
// @param populations the populations in a row-col-pop format
// @param dispersionCoefficients the coefficients of dispersion for each population
void computePopulationsDispersion(vector<vector<vector<double>>> & populations, vector<double> dispersionCoefficients) {
    // transform row-col-pop to pop-row-col
    vector<vector<vector<double>>> populationsTransformed = vector(populations[0][0].size(), vector(populations.size(), vector(populations[0].size(), 0.0)));
    for (int pop = 0; pop < populations[0][0].size(); pop++) {
        for (int row = 0; row < populations.size(); row++) {
            for (int col = 0; col < populations[0].size(); col++) {
                populationsTransformed[pop][row][col] = populations[row][col][pop];
            }
        }
    }

    // compute dispersion increment for every population
    for (int pop = 0; pop < populations[0][0].size(); pop++) {
        if (diffusion_method == DIFFUSION_ADI) {
            populationsTransformed[pop] = computeSinglePopulationDispersionADI(populationsTransformed[pop], static_cast<double>(dispersionCoefficients[pop]));
        } else {
            if (boundary_condition == BC_NEUMANN) {
                // Neumann explicit increment scaled by D
                auto inc = computeSinglePopulationDispersionExplicitNeumann(populationsTransformed[pop]);
                for (int y = 0; y < (int)inc.size(); ++y) {
                    for (int x = 0; x < (int)inc[0].size(); ++x) {
                        inc[y][x] = static_cast<int>(inc[y][x] * dispersionCoefficients[pop]);
                    }
                }
                populationsTransformed[pop] = std::move(inc);
            } else {
                populationsTransformed[pop] = computeSinglePopulationDispersion(populationsTransformed[pop], dispersionCoefficients[pop]);
            }
        }
    }

    // transform pop-row-col to row-col-pop and add increment
    for (int pop = 0; pop < (int)populationsTransformed.size(); pop++) {
        for (int row = 0; row < (int)populationsTransformed[0].size(); row++) {
            for (int col = 0; col < (int)populationsTransformed[0][0].size(); col++) {
                populations[row][col][pop] = std::max(populations[row][col][pop] + populationsTransformed[pop][row][col], 0.0);
            }
        }
    }
}

// computes the dispersion for a single population
// @param population the population in a row-col format
// @param dispersionCoefficient the coefficient of dispersion
// @return the dispersed population
vector<vector<double>> computeSinglePopulationDispersion(vector<vector<double>> population, double dispersionCoefficient) {
    // compute the 1D dispersion for every row
    // newPopulationCols is in a row-col format
    vector<vector<double>> newPopulationRows = vector(population.size(), vector(population[0].size(), 0.0));
    for (int row = 0; row < population.size(); row++) {
        newPopulationRows[row] = computePopulation1DimDispersion(population[row]);
    }

    // compute the 1D dispersion for every column
    // newPopulationCols is in a col-row format
    vector<vector<double>> newPopulationCols = vector(population[0].size(), vector(population.size(), 0.0));
    for (int col = 0; col < population[0].size(); col++) {
        vector<double> populationCol = vector(population.size(), 0.0);
        for (int row = 0; row < population.size(); row++) {
            populationCol[row] = population[row][col];
        }
        newPopulationCols[col] = computePopulation1DimDispersion(populationCol);
    }

    // combine the 1D dispersions to yield the 2D dispersion
    vector<vector<double>> newPopulation = vector(population.size(), vector(population[0].size(), 0.0));
    for (int row = 0; row < population.size(); row++) {
        for (int col = 0; col < population[0].size(); col++) {
            newPopulation[row][col] = (newPopulationRows[row][col] + newPopulationCols[col][row]) * dispersionCoefficient;
        }
    }
    return newPopulation;
}

// --- ADI (Crank–Nicolson) diffusion producing increments (u^{n+1} - u^n) ---
static std::vector<std::vector<double>> computeSinglePopulationDispersionADI(const std::vector<std::vector<double>>& population, double diffusionCoefficient) {
    const int H = static_cast<int>(population.size());
    const int W = H > 0 ? static_cast<int>(population[0].size()) : 0;
    if (H == 0 || W == 0) return {};

    // Convert to double for numerics
    std::vector<std::vector<double>> u = population;

    const double dt = delta_time; // configurable
    const double h2 = 1.0;  // grid spacing squared
    const double r = (diffusionCoefficient * dt) / (2.0 * h2);

    // First half-step: (I - r T_x) U* = (I + r T_y) U^n
    static std::vector<double> a, b, c, rhs;
    a.resize(std::max(W, H));
    b.resize(std::max(W, H));
    c.resize(std::max(W, H));
    rhs.resize(std::max(W, H));
    std::vector<std::vector<double>> u_star(H, std::vector<double>(W, 0.0));

    // Pre-build x-direction system coefficients (depends only on W and r)
    if (boundary_condition == BC_NEUMANN) build_tridiagonal_neumann(W, r, a, b, c);
    else build_tridiagonal_dirichlet(W, r, a, b, c);

    for (int y = 0; y < H; ++y) {
        // RHS = (I + r T_y) applied to u at (y, x)
        for (int x = 0; x < W; ++x) {
            double ly;
            if (boundary_condition == BC_NEUMANN) {
                ly = laplace_neumann_y(u, y, x);
            } else {
                double up = (y > 0) ? u[y - 1][x] : 0.0;
                double down = (y < H - 1) ? u[y + 1][x] : 0.0;
                ly = up - 2.0 * u[y][x] + down;
            }
            rhs[x] = u[y][x] + r * ly;
        }
        // Solve (I - r T_x) row system
        std::vector<double> aa(a.begin(), a.begin() + W);
        std::vector<double> bb(b.begin(), b.begin() + W);
        std::vector<double> cc(c.begin(), c.begin() + W);
        std::vector<double> dd(rhs.begin(), rhs.begin() + W);
        thomas_solve_inplace(aa, bb, cc, dd);
        for (int x = 0; x < W; ++x) u_star[y][x] = dd[x];
    }

    // Second half-step: (I - r T_y) U^{n+1} = (I + r T_x) U*
    // Pre-build y-direction coefficients
    if (boundary_condition == BC_NEUMANN) build_tridiagonal_neumann(H, r, a, b, c);
    else build_tridiagonal_dirichlet(H, r, a, b, c);
    std::vector<std::vector<double>> u_next(H, std::vector<double>(W, 0.0));
    for (int x = 0; x < W; ++x) {
        // RHS = (I + r T_x) applied to u_star along x
        for (int y = 0; y < H; ++y) {
            double lx;
            if (boundary_condition == BC_NEUMANN) {
                lx = laplace_neumann_x(u_star, y, x);
            } else {
                double left = (x > 0) ? u_star[y][x - 1] : 0.0;
                double right = (x < W - 1) ? u_star[y][x + 1] : 0.0;
                lx = left - 2.0 * u_star[y][x] + right;
            }
            rhs[y] = u_star[y][x] + r * lx;
        }
        // Solve (I - r T_y) column system
        std::vector<double> aa(a.begin(), a.begin() + H);
        std::vector<double> bb(b.begin(), b.begin() + H);
        std::vector<double> cc(c.begin(), c.begin() + H);
        std::vector<double> dd(rhs.begin(), rhs.begin() + H);
        thomas_solve_inplace(aa, bb, cc, dd);
        for (int y = 0; y < H; ++y) u_next[y][x] = dd[y];
    }

    // Return diffusion increment: u^{n+1} - u^{n}
    std::vector<std::vector<double>> increment(H, std::vector<double>(W, 0.0));
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            increment[y][x] = u_next[y][x] - u[y][x];
        }
    }
    return increment;
}

// explicit diffusion increment with Neumann BC (zero flux)
static std::vector<std::vector<double>> computeSinglePopulationDispersionExplicitNeumann(const std::vector<std::vector<double>>& population) {
    const int H = static_cast<int>(population.size());
    const int W = H > 0 ? static_cast<int>(population[0].size()) : 0;
    std::vector<std::vector<double>> inc(H, std::vector<double>(W, 0.0));
    if (H == 0 || W == 0) return inc;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            double up = (y > 0) ? population[y - 1][x] : population[y + 1][x];
            double down = (y < H - 1) ? population[y + 1][x] : population[y - 1][x];
            double left = (x > 0) ? population[y][x - 1] : population[y][x + 1];
            double right = (x < W - 1) ? population[y][x + 1] : population[y][x - 1];
            inc[y][x] = (up + down + left + right) - 4.0 * population[y][x];
        }
    }
    return inc;
}

static inline double laplace_neumann_y(const std::vector<std::vector<double>>& u, int y, int x) {
    const int H = static_cast<int>(u.size());
    // Neumann: mirror at boundaries
    double up = (y > 0) ? u[y - 1][x] : u[y + 1][x];
    double down = (y < H - 1) ? u[y + 1][x] : u[y - 1][x];
    return up - 2.0 * u[y][x] + down;
}

static inline double laplace_neumann_x(const std::vector<std::vector<double>>& u, int y, int x) {
    const int W = static_cast<int>(u[0].size());
    double left = (x > 0) ? u[y][x - 1] : u[y][x + 1];
    double right = (x < W - 1) ? u[y][x + 1] : u[y][x - 1];
    return left - 2.0 * u[y][x] + right;
}

static void build_tridiagonal_neumann(int n, double r, std::vector<double>& a, std::vector<double>& b, std::vector<double>& c) {
    a.assign(n, 0.0);
    b.assign(n, 0.0);
    c.assign(n, 0.0);
    // For (I - r T) with T having Neumann-modified ends: off-diagonal magnitude doubles at boundaries
    // Internal nodes: diag = 1 + 2r, off = -r
    for (int i = 0; i < n; ++i) {
        b[i] = 1.0 + 2.0 * r;
        if (i > 0) a[i] = -r;
        if (i < n - 1) c[i] = -r;
    }
    // Neumann adjustment: first and last off-diagonal doubled in magnitude (because T boundary off-diag is 2)
    if (n >= 2) {
        c[0] = -2.0 * r;
        a[n - 1] = -2.0 * r;
    }
}

static void build_tridiagonal_dirichlet(int n, double r, std::vector<double>& a, std::vector<double>& b, std::vector<double>& c) {
    a.assign(n, 0.0);
    b.assign(n, 0.0);
    c.assign(n, 0.0);
    for (int i = 0; i < n; ++i) {
        b[i] = 1.0 + 2.0 * r;
        if (i > 0) a[i] = -r;
        if (i < n - 1) c[i] = -r;
    }
    // Dirichlet BC: nothing special beyond standard tri-diagonal; boundaries remain as set
}

static void thomas_solve_inplace(std::vector<double>& a, std::vector<double>& b, std::vector<double>& c, std::vector<double>& d) {
    // a: sub, b: diag, c: super, d: RHS; all size n; a[0] ignored, c[n-1] ignored
    const int n = static_cast<int>(b.size());
    for (int i = 1; i < n; ++i) {
        double m = a[i] / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
    }
    d[n - 1] /= b[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        d[i] = (d[i] - c[i] * d[i + 1]) / b[i];
    }
}

// computes the dispersion in 1D
// @param populationVec the vector representing the population in 1D
// @return the dispersed 1D population
vector<double> computePopulation1DimDispersion(vector<double> populationVec) {
    const int n = static_cast<int>(populationVec.size());
    if (n <= 1) return populationVec;
    vector<double> result(n, 0.0);
    // Dirichlet-style (as original): at ends, use single neighbor
    // result[i] = u[i-1] - 2u[i] + u[i+1]
    // left boundary
    result[0] = -2.0 * populationVec[0] + populationVec[1];
    for (int i = 1; i < n - 1; ++i) {
        result[i] = populationVec[i - 1] - 2.0 * populationVec[i] + populationVec[i + 1];
    }
    // right boundary
    result[n - 1] = populationVec[n - 2] - 2.0 * populationVec[n - 1];
    return result;
}

// generates the dispersion matrix to perform a 1D dispersion
// @param n the dimension of the matrix (the matrix will always be quadratic)
// @return the dispersion matrix
vector<vector<double>> generateDispersionMatrix(int n) {
    auto dispersionMatrix = vector(n, vector(n, 0.0));
    for (int i = 0; i < n; i++) {
        if (i == 0) {
            dispersionMatrix[i][i] = -2.0;
            dispersionMatrix[i][i + 1] = 1.0;
        } else if (i == n - 1) {
            dispersionMatrix[i][i - 1] = 1.0;
            dispersionMatrix[i][i] = -2.0;
        } else {
            dispersionMatrix[i][i - 1] = 1.0;
            dispersionMatrix[i][i] = -2.0;
            dispersionMatrix[i][i + 1] = 1.0;
        }
    }
    return dispersionMatrix;
}

// performs a matrix-vector multiplication
// @param vec the vector in the multiplication
// @param matrix the matrix in the multiplication
// @return the result of the multiplication
vector<double> matrixVectorMultiplication(vector<double> vec, vector<vector<double>> matrix) {
    vector<double> resultVector(vec.size(), 0.0);
    for (int row = 0; row < matrix.size(); row++) {
        double sum = 0.0;
        if (row > 0) {
            sum += vec[row - 1] * matrix[row][row - 1]; // left diagonal element
        }
        sum += vec[row] * matrix[row][row]; // main diagonal element
        if (row < matrix.size() - 1) {
            sum += vec[row + 1] * matrix[row][row + 1]; // right diagonal element
        }
        resultVector[row] = sum;
    }
    return resultVector;
}


// performs simple scalar-product
// @param animals the vector containing the population of animals at this cell
// @param coefficients the vector expressing the influence of other populations on our current
float scalarProduct(vector<double> & animals, vector<double> & coefficients) {
    double res = 0.0;
    for (int i = 0; i < animals.size(); i++) {
        res += animals[i]*coefficients[i];
    }
    return static_cast<float>(res);
}


// performs the population change on all cells
// @param board is the whole board with populations
// @param coefficients is how animal at index i is influenced from other population
void computeChangedPopulation(vector<vector<vector<double>>> & board, vector<vector<double>> & coefficients) {
    // dp/dt = coef * arr
    for (int y = 0; y < board.size(); y++) {
        for (int x = 0; x < board[0].size(); x++) {
            for (int k = 0; k < board[0][0].size(); k++) {
                board[y][x][k] += static_cast<double>(scalarProduct(board[y][x], coefficients[k]));
            }
        }
    }
}

// convert [y][x][k] to [k][y][x]
void add_to_steps() {
    vector<vector<vector<double>>> res = vector(species.size(), vector<vector<double>>(board.size(), vector<double>(board[0].size(),0.0)));
    for (int y = 0; y < board.size(); y++) {
        for (int x = 0; x < board[0].size(); x++) {
            for (int k = 0; k < species.size(); k++) {
                res[k][y][x] = board[y][x][k];
            }
        }
    }
    steps.push_back(res);
}

void prepareCalculations() {
    // baseline run according to current method
    steps.clear();
    add_to_steps();
    for (int i = 0; i < number_steps_t; i++) {
        computeChangedPopulation(board, coefficients);
        computePopulationsDispersion(board, dispersion_coefficients);
        add_to_steps();
    }

    if (compare_methods) {
        // clone initial board
        auto board0 = steps[0]; // [k][y][x]
        // explicit
        steps_explicit.clear();
        steps_explicit.push_back(board0);
        // reconstruct board as [y][x][k]
        vector<vector<vector<double>>> board_exp(board_height, vector<vector<double>>(board_width, vector<double>(species.size(), 0.0)));
        for (int y = 0; y < board_height; ++y) {
            for (int x = 0; x < board_width; ++x) {
                for (int k = 0; k < species.size(); ++k) {
                    board_exp[y][x][k] = board0[k][y][x];
                }
            }
        }
        auto saved_method = diffusion_method;
        diffusion_method = DIFFUSION_EXPLICIT;
        for (int t = 0; t < number_steps_t; ++t) {
            computeChangedPopulation(board_exp, coefficients);
            computePopulationsDispersion(board_exp, dispersion_coefficients);
            // push [k][y][x]
            vector<vector<vector<double>>> res(species.size(), vector<vector<double>>(board_height, vector<double>(board_width, 0.0)));
            for (int y = 0; y < board_height; ++y)
                for (int x = 0; x < board_width; ++x)
                    for (int k = 0; k < species.size(); ++k)
                        res[k][y][x] = board_exp[y][x][k];
            steps_explicit.push_back(std::move(res));
        }

        // ADI
        steps_adi.clear();
        steps_adi.push_back(board0);
        vector<vector<vector<double>>> board_adi = board_exp; // start from same initial
        diffusion_method = DIFFUSION_ADI;
        for (int t = 0; t < number_steps_t; ++t) {
            computeChangedPopulation(board_adi, coefficients);
            computePopulationsDispersion(board_adi, dispersion_coefficients);
            vector<vector<vector<double>>> res(species.size(), vector<vector<double>>(board_height, vector<double>(board_width, 0.0)));
            for (int y = 0; y < board_height; ++y)
                for (int x = 0; x < board_width; ++x)
                    for (int k = 0; k < species.size(); ++k)
                        res[k][y][x] = board_adi[y][x][k];
            steps_adi.push_back(std::move(res));
        }
        diffusion_method = saved_method;
    }
}

