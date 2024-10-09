#ifndef SETTINGS
#define SETTINGS

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


inline ImVec4 HexToImVec4(uint32_t hex) {
    float r = ((hex >> 24) & 0xFF) / 255.0f;
    float g = ((hex >> 16) & 0xFF) / 255.0f;
    float b = ((hex >> 8) & 0xFF) / 255.0f;
    float a = (hex & 0xFF) / 255.0f;
    return {r, g, b, a};
}

inline State current = CONFIGURATION;
inline int board_width = 10;
inline int board_height = 10;
// int because it represents for all 15 species its count of them in every block
inline vector<vector<vector<int>>> board(board_height, vector<vector<int>>(board_width, vector<int>(15, 0)));
inline vector<Species*> species;
inline vector<vector<float>> coefficients(15, vector<float>(15, 0.0f));
inline vector<float> dispersion_coefficients(15, 0.4f);

inline int selected_box = -1; // not initalised



#endif