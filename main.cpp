#include <imgui_internal.h>
#include "imgui.h"
#include <glad/glad.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "third_party/Species.h"
using namespace std;

#define CANVAS_WIDTH 1320
#define CANVAS_HEIGHT  720
#define BOARD_LIMIT  20
#define SPECIES_LIMIT 15

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
    0x000000FF, // Black
    0xFFFFFFFF, // White
    0x8B0000FF, // Dark Red
    0x2E8B57FF, // Sea Green
    0x4682B4FF  // Steel Blue
};
State current = CONFIGURATION;

int board_width = 10;
int board_height = 10;
vector<vector<Block*>> board(board_height, vector<Block*>(15, new Block()));
vector<Species*> species;



void config_board_size() {
    int board_width_slider = board_width;
    if (ImGui::SliderInt("Board Width", &board_width_slider, 2, BOARD_LIMIT)) {
        if (board_width_slider > board_width) {
            // add board_width_slider - board_width els to every arr
            for (auto & i : board) {
                for (int j = 0; j < board_width_slider-board_width; j++) {
                    i.push_back(new Block());
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
    if (ImGui::SliderInt("Board Height", &board_height_slider, 2, BOARD_LIMIT)) {
        if (board_height_slider > board_height) {
            // add board_height_slider - board_height rows
            const vector<Block*> row(board_width, new Block());
            for (int j = 0; j < board_width_slider-board_width; j++) {
                board.push_back(row);
            }
        } else {
            // remove board_height - board_height_slider rows
            for (int j = 0; j < board_width-board_width_slider; j++) {
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
        ImGui::Text(species[i]->name.c_str());
    }
}


void board_render() {
    // width height == 500
    double size_x = 490.0 / board_width;
    double size_y = 490.0 / board_height;

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
                if (ImGui::Button(("##Button" + std::to_string(row * board_width + col)).c_str(), buttonSize)) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
                    ImGui::PopStyleColor(); // Restore color if modified
                }
            }
        }
        ImGui::EndTable();
    }
    style.CellPadding = originalCellPadding;

}
void renderConfiguration() {

    ImGui::SetNextWindowPos(ImVec2(100, 100));
    ImGui::SetNextWindowSize(ImVec2(300, 200));
    ImGui::Begin("Game Configurations", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    // Create the slider and check if the value has changed
    config_board_size();
    config_board_species();

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(420, 100));
    ImGui::SetNextWindowSize(ImVec2(300, 200));
    ImGui::Begin("Species/Cells", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    config_species_list();
    ImGui::End();


    ImGui::SetNextWindowPos(ImVec2(100, 320));
    ImGui::SetNextWindowSize(ImVec2(620, 200));
    ImGui::Begin("Dynamics Between Species/Cells", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Rendering..");
    if (ImGui::Button("Simulate")) {
        current = SIMULATION;
    }
    ImGui::End();


    // board
    ImGui::SetNextWindowPos(ImVec2(740, 100));
    ImGui::SetNextWindowSize(ImVec2(500, 500));
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
