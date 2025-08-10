#include "Presets.h"
#include "settings.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

struct PresetDef {
    std::string name;
    std::string desc;
    std::vector<std::string> species_names;
    std::vector<uint32_t> species_colors;
    std::vector<std::vector<double>> A; // interaction matrix [affected][source]
    std::vector<double> D;              // dispersion per species
    BoundaryCondition bc;
    DiffusionMethod method;
    double dt;
    int steps;
    std::function<void()> init_board;   // fills global board
};

static std::vector<PresetDef> g_presets;
static std::vector<std::string> g_names;

static void set_species(const std::vector<std::string>& names, const std::vector<uint32_t>& cols) {
    species.clear();
    for (int i = 0; i < (int)names.size(); ++i) {
        species.emplace_back(names[i], cols[i % (int)cols.size()]);
        species.back().color = cols[i % (int)cols.size()];
    }
}

static void set_coefficients(const std::vector<std::vector<double>>& A) {
    const int s = (int)A.size();
    coefficients.assign(15, std::vector<double>(15, 0.0));
    for (int i = 0; i < s; ++i) for (int j = 0; j < s; ++j) coefficients[i][j] = A[i][j];
}

static void set_dispersion(const std::vector<double>& Dvec) {
    dispersion_coefficients.assign(15, 0.0);
    for (int i = 0; i < (int)Dvec.size(); ++i) dispersion_coefficients[i] = Dvec[i];
}

static void seed_gaussian(int cx, int cy, double amp, double sigma, int k) {
    for (int y = 0; y < board_height; ++y) {
        for (int x = 0; x < board_width; ++x) {
            double dx = x - cx;
            double dy = y - cy;
            board[y][x][k] += amp * std::exp(-(dx*dx + dy*dy) / (2.0 * sigma * sigma));
        }
    }
}

static void seed_gaussian_frac(double fx, double fy, double amp, double sigma_frac, int k) {
    int cx = std::clamp((int)std::round(fx * (board_width - 1)), 0, board_width - 1);
    int cy = std::clamp((int)std::round(fy * (board_height - 1)), 0, board_height - 1);
    double sigma = sigma_frac * std::max(1, std::min(board_width, board_height));
    seed_gaussian(cx, cy, amp, sigma, k);
}

static void seed_ring_frac(double fx, double fy, double r_inner_frac, double r_outer_frac, double val, int k) {
    int cx = std::clamp((int)std::round(fx * (board_width - 1)), 0, board_width - 1);
    int cy = std::clamp((int)std::round(fy * (board_height - 1)), 0, board_height - 1);
    double scale = std::max(1, std::min(board_width, board_height));
    double r1 = std::max(1.0, r_inner_frac * scale);
    double r2 = std::max(r1+1.0, r_outer_frac * scale);
    for (int y=0;y<board_height;++y) {
        for (int x=0;x<board_width;++x) {
            double dx=x-cx, dy=y-cy; double d2 = dx*dx+dy*dy;
            if (d2 >= r1*r1 && d2 <= r2*r2) board[y][x][k] = val;
        }
    }
}

void init_presets() {
    if (!g_presets.empty()) return;

    // Fresh curated presets respecting current board size

    // 1) Two-Front Collision (Dirichlet)
    g_presets.push_back({
        "Two-Front Collision",
        "Single species invading from left and right, colliding in the center",
        {"ink"}, {colors[10]},
        {{-0.005}},
        {0.12},
        BC_DIRICHLET,
        DIFFUSION_EXPLICIT,
        0.8,
        200,
        [](){
            int w = std::max(1, (int)std::round(0.08 * board_width));
            for (int y=0;y<board_height;++y) {
                for (int x=0;x<w; ++x) board[y][x][0]=50.0;
                for (int x=board_width-w; x<board_width; ++x) if (x>=0) board[y][x][0]=50.0;
            }
        }
    });

    // 2) Predator Core vs Prey Ring (Neumann, ADI)
    g_presets.push_back({
        "Predator Core vs Prey Ring",
        "Prey ring around a predator core; waves form as predator chases prey",
        {"prey","predator"}, {colors[0], colors[1]},
        {{+0.02,-0.04},{+0.06,-0.05}},
        {0.05,0.09},
        BC_NEUMANN,
        DIFFUSION_ADI,
        1.0,
        220,
        [](){
            seed_ring_frac(0.5,0.5, 0.16, 0.24, 100.0, 0); // prey ring mass
            seed_gaussian_frac(0.5,0.5, 150.0, 0.05, 1);   // predator core mass
        }
    });

    // 3) Cyclic Triad Waves (Neumann, ADI)
    g_presets.push_back({
        "Cyclic Triad Waves",
        "Rock–paper–scissors waves seeded at three vertices",
        {"R","P","S"}, {colors[0], colors[7], colors[9]},
        {{-0.02,+0.03,-0.03},{-0.03,-0.02,+0.03},{+0.03,-0.03,-0.02}},
        {0.06,0.06,0.06},
        BC_NEUMANN,
        DIFFUSION_ADI,
        1.0,
        240,
        [](){
            seed_gaussian_frac(0.20,0.20, 120.0, 0.05, 0);
            seed_gaussian_frac(0.80,0.25, 120.0, 0.05, 1);
            seed_gaussian_frac(0.50,0.80, 120.0, 0.05, 2);
        }
    });

    // 4) Competitive Domains From Noise (Neumann, Explicit)
    g_presets.push_back({
        "Competitive Domains From Noise",
        "Two rivals segregate into domains starting from tiny noise",
        {"blue","yellow"}, {colors[0], colors[2]},
        {{-0.02,-0.03},{-0.03,-0.02}},
        {0.02,0.02},
        BC_NEUMANN,
        DIFFUSION_EXPLICIT,
        0.7,
        300,
        [](){
            for (int y=0;y<board_height;++y) for (int x=0;x<board_width;++x) {
                board[y][x][0] = (rand()%1000)/33.0; // ~0..30
                board[y][x][1] = (rand()%1000)/33.0;
            }
        }
    });

    // 5) Four Corners Convergence (Dirichlet, Explicit)
    g_presets.push_back({
        "Four Corners Convergence",
        "Four species seeded at corners converge and interact",
        {"A","B","C","D"}, {colors[0], colors[8], colors[12], colors[5]},
        {{-0.005,0,0,0},{0,-0.005,0,0},{0,0,-0.005,0},{0,0,0,-0.005}},
        {0.08,0.08,0.08,0.08},
        BC_DIRICHLET,
        DIFFUSION_EXPLICIT,
        0.6,
        180,
        [](){
            int m = std::max(2, std::min(board_width, board_height)/10);
            for (int y=0;y<m;++y) for (int x=0;x<m;++x) board[y][x][0]=200.0;
            for (int y=board_height-m;y<board_height;++y) for (int x=0;x<m;++x) board[y][x][1]=200.0;
            for (int y=0;y<m;++y) for (int x=board_width-m;x<board_width;++x) board[y][x][2]=200.0;
            for (int y=board_height-m;y<board_height;++y) for (int x=board_width-m;x<board_width;++x) board[y][x][3]=200.0;
        }
    });

    // 6) Fast vs Slow Diffusion Twins (Neumann, ADI)
    g_presets.push_back({
        "Fast vs Slow Diffusion Twins",
        "Two species with identical initial blobs but very different diffusion",
        {"slow","fast"}, {colors[11], colors[5]},
        {{0.0,0.0},{0.0,0.0}},
        {0.01,0.18},
        BC_NEUMANN,
        DIFFUSION_ADI,
        1.0,
        200,
        [](){ seed_gaussian_frac(0.5, 0.5, 200.0, 0.10, 0); seed_gaussian_frac(0.5, 0.5, 200.0, 0.10, 1);} 
    });

    g_names.clear();
    for (auto& p: g_presets) g_names.push_back(p.name);
}

const std::vector<std::string>& get_preset_names() { return g_names; }

void apply_preset(int index) {
    if (index < 0 || index >= (int)g_presets.size()) return;
    const auto& p = g_presets[index];
    set_species(p.species_names, p.species_colors);
    set_coefficients(p.A);
    set_dispersion(p.D);
    boundary_condition = p.bc;
    diffusion_method = p.method;
    delta_time = p.dt;
    number_steps_t = p.steps;
    // initialize board
    for (int y=0;y<board_height;++y) for (int x=0;x<board_width;++x) for (int k=0;k<(int)p.species_names.size();++k) board[y][x][k]=0.0;
    p.init_board();
}


