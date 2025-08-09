#ifndef SETTINGS
#define SETTINGS

#include "imgui.h"
#include <GLFW/glfw3.h>
#include <vector>
#include "./Species.h"

#define CANVAS_WIDTH 1320
#define CANVAS_HEIGHT  820
#define BOARD_LIMIT  20
#define SPECIES_LIMIT 5
#define PADDING 50
#define WINDOW_SPACING 10

#define CONFIG_WINDOW_WIDTH ((CANVAS_WIDTH - 2*PADDING - WINDOW_SPACING*2)/4)
#define CONFIG_WINDOW_HEIGHT 200
#define BOARD_WINDOW_WIDTH ((CANVAS_WIDTH - 2*PADDING - WINDOW_SPACING*2)/2)
#define BOARD_WINDOW_HEIGHT 700


using namespace std;

inline uint32_t colors[15] = {
    0xFF0000FF, // Red
    0x00FF00FF, // Green
    0xFFFF00FF, // Yellow
    0x32CD32FF, // Lime Green
    0xADD8E6FF, // Light Blue
    0xFF00FFFF, // Cyan
    0x808080FF, // Gray
    0xFFA500FF, // Orange
    0x800080FF, // Purple
    0x008080FF, // Teal
    0xFF69B4FF, // Pink
    0xFFFFFFFF, // White
    0x8B0000FF, // Dark Red
    0x2E8B57FF, // Sea Green
    0x4682B4FF  // Steel Blue
};

enum State {
    CONFIGURATION,
    SIMULATION
};

// Diffusion method selection
enum DiffusionMethod {
    DIFFUSION_EXPLICIT = 0,
    DIFFUSION_ADI = 1
};

// Boundary condition selection
enum BoundaryCondition {
    BC_DIRICHLET = 0, // value outside domain is 0
    BC_NEUMANN = 1    // zero-flux, mirror at boundary
};


inline ImVec4 HexToImVec4(uint32_t hex) {
    float r = ((hex >> 24) & 0xFF) / 255.0f;
    float g = ((hex >> 16) & 0xFF) / 255.0f;
    float b = ((hex >> 8) & 0xFF) / 255.0f;
    float a = (hex & 0xFF) / 255.0f;
    return {r, g, b, a};
}

inline State current = CONFIGURATION;
inline DiffusionMethod diffusion_method = DIFFUSION_EXPLICIT;
inline BoundaryCondition boundary_condition = BC_DIRICHLET;
inline double delta_time = 1.0; // simulation time step
inline int board_width = 10;
inline int board_height = 10;
// int because it represents for all 15 species its count of them in every block
inline vector<vector<vector<double>>> board(board_height, vector<vector<double>>(board_width, vector<double>(15, 0.0)));
inline vector<Species> species;
inline vector<vector<double>> coefficients(15, vector<double>(15, 0.0));
inline vector<double> dispersion_coefficients(15, 0.1);

inline int selected_box = -1; // not initalised
inline int number_steps_t = 10;

// 5 d, [step][population][y][x]
inline vector<vector<vector<vector<double>>>> steps;
inline vector<vector<vector<vector<double>>>> steps_explicit;
inline vector<vector<vector<vector<double>>>> steps_adi;

inline bool compare_methods = false;


#endif