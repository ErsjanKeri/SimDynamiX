#include <imgui_internal.h>
#include "imgui.h"
#include <glad/glad.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <sstream>

#include "third_party/Species.h"
using namespace std;

#define CANVAS_WIDTH 1320
#define CANVAS_HEIGHT  820
#define BOARD_LIMIT  20
#define SPECIES_LIMIT 15
#define PADDING 50
#define WINDOW_SPACING 10

#define CONFIG_WINDOW_WIDTH ((CANVAS_WIDTH - 2*PADDING - WINDOW_SPACING*2)/4)
#define CONFIG_WINDOW_HEIGHT 200
#define BOARD_WINDOW_WIDTH ((CANVAS_WIDTH - 2*PADDING - WINDOW_SPACING*2)/2)
#define BOARD_WINDOW_HEIGHT 700

// TODO, between 1 - 20 tiles
// f(x,y,t) is actually density function at t


enum State {
    CONFIGURATION,
    SIMULATION
};

uint32_t colors[15] = {
    0xFF0000FF, // Red
    0x00FF00FF, // Green
    0x0000FFFF, // Blue
    0xFFFFFF00, // Yellow
    0xFFFF00FF, // Magenta
    0xFF00FFFF, // Cyan
    0x808080FF, // Gray
    0xFFA500FF, // Orange
    0x800080FF, // Purple
    0x008080FF, // Teal
    0xFF69B4FF, // Black
    0xFFFFFFFF, // White
    0x8B0000FF, // Dark Red
    0x2E8B57FF, // Sea Green
    0x4682B4FF  // Steel Blue
};

ImVec4 HexToImVec4(uint32_t hex) {
    float r = ((hex >> 24) & 0xFF) / 255.0f;
    float g = ((hex >> 16) & 0xFF) / 255.0f;
    float b = ((hex >> 8) & 0xFF) / 255.0f;
    float a = (hex & 0xFF) / 255.0f;
    return ImVec4(r, g, b, a);
}

State current = CONFIGURATION;

int board_width = 10;
int board_height = 10;
// int because it represents for all 15 species its count of them in every block
vector<vector<vector<int>>> board(board_height, vector<vector<int>>(board_width, vector<int>(15, 0)));
vector<Species*> species;
int selected_box = -1; // not initalised


void config_board_size() {
    int board_width_slider = board_width;
    if (ImGui::SliderInt("Board Width", &board_width_slider, 1, BOARD_LIMIT)) {
        if (board_width_slider > board_width) {
            // add board_width_slider - board_width els to every arr
            for (auto & i : board) {
                for (int j = 0; j < board_width_slider-board_width; j++) {
                    i.emplace_back(15,0);
                }
            }
        } else {
            // remove board_width - board_width_slider elements
            for (auto & i : board) {
                for (int j = 0; j < board_width-board_width_slider; j++) {
                    i.pop_back();
                }
            }
        }
        board_width = board_width_slider;
        cout << board[0].size() << "\n";
    }
    int board_height_slider = board_height;
    if (ImGui::SliderInt("Board Height", &board_height_slider, 1, BOARD_LIMIT)) {
        if (board_height_slider > board_height) {
            // add board_height_slider - board_height rows
            for (int j = 0; j < board_height_slider-board_height; j++) {
                board.emplace_back(board_width, vector<int>(15,0));
            }
        } else {
            // remove board_height - board_height_slider rows
            for (int j = 0; j < board_height_slider-board_height; j++) {
                board.pop_back();
            }
        }
        board_height = board_height_slider;
        cout << board.size() << "\n";
    }
}

void config_board_species(){

    u_int counter = species.size();
    ImGui::BeginDisabled(counter < 2); // cant get below 1
    if (ImGui::Button("-")) {
        species.pop_back();  // Decrement the counter when the minus button is clicked
    }
    ImGui::EndDisabled();

    ImGui::SameLine();  // Keep the counter value on the same line as the buttons
    ImGui::Text("%d", counter);
    ImGui::SameLine();  // Keep the plus button on the same line
    ImGui::BeginDisabled(counter > 14); // cant get below 1
    if (ImGui::Button("+")) {
        species.push_back(new Species("specie " + std::to_string(counter+1), colors[counter]));  // Increment the counter when the plus button is clicked
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("Number of Species");
}

void config_species_list() {
    for (int i = 0; i < species.size(); i++) {
        // TODO, check to prevent string getting deleted, problem is the last character
        ImGui::PushStyleColor(ImGuiCol_Text, HexToImVec4(colors[i]));
        ImGui::InputText(("##species"+to_string(i)).c_str(), species[i]->name.data(), 128);
        ImGui::PopStyleColor(1);
    }
}

void config_dynamics() {

    ImGui::Text("Rendering..");
    if (ImGui::Button("Simulate")) {
        current = SIMULATION;
    }
}

void board_render() {
    // width height == 500
    double size_x = (BOARD_WINDOW_WIDTH-65) / board_width;
    double size_y = (BOARD_WINDOW_WIDTH-65) / board_height;

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 originalCellPadding = style.CellPadding;     // Save original cell padding
    // set bot to 0 to remove padding between rows
    style.CellPadding = ImVec2(1, 1);

    // ImGuiTableFlags_NoPadInnerX to remove padding between columns
    if (ImGui::BeginTable("Grid Table", board_width, ImGuiTableFlags_SizingFixedFit )) {
        ImVec2 buttonSize = ImVec2(size_x, size_y);  // Customize the button size here

        for (int row = 0; row < board_height; row++) {
            ImGui::TableNextRow();  // Move to the next row
            for (int col = 0; col < board_width; col++) {
                ImGui::TableSetColumnIndex(col);  // Set the current column

                ostringstream oss;

                // if (selected_box != -1 && row == selected_box/board_width && col == selected_box%board_width) {
                    for (size_t i = 0; i < species.size(); ++i) {
                        oss << board[row][col][i];
                        if (i < species.size()-1) {
                            oss<<",";
                        }
                    }
                // }
                oss << " ##Button"+to_string(row * board_width + col);
                if (ImGui::Button(oss.str().c_str(), buttonSize)) {
                    selected_box = row*board_width+col;
                }

            }
        }
        ImGui::EndTable();
        ImGui::Spacing();
        int x = selected_box%board_width;
        int y = selected_box/board_width;
        if (selected_box != -1) {
            ImGui::Text(("Population inside square X: "
                    + to_string(x) + ", Y: "
                    + to_string(y)).c_str());
        } else {
            ImGui::Text("Population inside square");
        }
        ImGui::BeginDisabled(selected_box == -1);
        for (int i = 0; i < species.size(); i++) {
            ImGui::PushStyleColor(ImGuiCol_Text, HexToImVec4(colors[i]));
            // checks needed so that in case of resizing, doesnt crash
            if (selected_box != -1 && y < board.size() && x < board[0].size()) {
                ImGui::InputInt(species[i]->name.c_str(), &board[y][x][i]);
            } else {
                int myInt = 0;
                ImGui::InputInt(species[i]->name.c_str(), &myInt);
            }
            ImGui::PopStyleColor(1);
        }
        ImGui::EndDisabled();

    }
    style.CellPadding = originalCellPadding;

}

void renderConfiguration() {

    ImGui::SetNextWindowPos(ImVec2(PADDING, PADDING));
    ImGui::SetNextWindowSize(ImVec2(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT));
    ImGui::Begin("Game Configurations", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    // Create the slider and check if the value has changed
    config_board_size();
    config_board_species();

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(PADDING+CONFIG_WINDOW_WIDTH+WINDOW_SPACING, PADDING));
    ImGui::SetNextWindowSize(ImVec2(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT));
    ImGui::Begin("Species/Cells", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    config_species_list();
    ImGui::End();


    ImGui::SetNextWindowPos(ImVec2(PADDING, PADDING + CONFIG_WINDOW_HEIGHT + WINDOW_SPACING));
    ImGui::SetNextWindowSize(ImVec2(CONFIG_WINDOW_WIDTH*2 + WINDOW_SPACING , CONFIG_WINDOW_HEIGHT));
    ImGui::Begin("Dynamics Between Species/Cells", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    config_dynamics();
    ImGui::End();


    // board
    ImGui::SetNextWindowPos(ImVec2(PADDING+2*(CONFIG_WINDOW_WIDTH+WINDOW_SPACING), PADDING));
    ImGui::SetNextWindowSize(ImVec2(BOARD_WINDOW_WIDTH , BOARD_WINDOW_HEIGHT));
    ImGui::Begin("Board", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    board_render();
    ImGui::End();

}

void renderSimulation() {
    ImGui::SetNextWindowPos(ImVec2(100, 100));
    ImGui::SetNextWindowSize(ImVec2(300, 200));

    ImGui::Begin("Rendering Simulation", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Rendering..");
    if (ImGui::Button("Stop Simulation")) {
        current = CONFIGURATION;
    }
    ImGui::End();
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // set up default species
    species.push_back(new Species("prey", colors[0]));
    species.push_back(new Species("predator", colors[1]));

    // Set OpenGL version to 4.1 Core (highest supported on macOS)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Core Profile

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(CANVAS_WIDTH, CANVAS_HEIGHT, "SimDynamiX", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    // IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Initialize ImGui for GLFW and OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");  // Use version 410 for OpenGL 4.1 Core


    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (current == CONFIGURATION) {
            renderConfiguration();
        } else {
            renderSimulation();
        }
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        // TODO, define as hex and use bit shift
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
