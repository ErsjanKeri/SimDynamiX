#include <imgui_internal.h>
#include "imgui.h"
#include "implot.h"
#include <glad/glad.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "third_party/Configuration.h"
#include "third_party/Simulation.h"

#include "third_party/Species.h"
#include "third_party/settings.h"

using namespace std;

void renderConfiguration() {

    ImGui::SetNextWindowPos(ImVec2(PADDING, PADDING));
    ImGui::SetNextWindowSize(ImVec2(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT));
    ImGui::Begin("Game Configurations", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    // Create the slider and check if the value has changed
    config_board_size_species();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(PADDING+CONFIG_WINDOW_WIDTH+WINDOW_SPACING, PADDING));
    ImGui::SetNextWindowSize(ImVec2(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT));
    ImGui::Begin("Species/Cells", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    config_species_list();
    ImGui::End();


    ImGui::SetNextWindowPos(ImVec2(PADDING, PADDING + CONFIG_WINDOW_HEIGHT + WINDOW_SPACING));
    ImGui::SetNextWindowSize(ImVec2(CONFIG_WINDOW_WIDTH*2 + WINDOW_SPACING , BOARD_WINDOW_HEIGHT-WINDOW_SPACING-CONFIG_WINDOW_HEIGHT));
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
    ImGui::SetNextWindowPos(ImVec2( PADDING, PADDING));
    ImGui::SetNextWindowSize(ImVec2(CANVAS_WIDTH-2*PADDING, CANVAS_HEIGHT-2*PADDING));
    ImGui::Begin("Rendering Simulation", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    simulations_list();

    if (ImGui::Button("Stop Simulation")) {
        // set the board to the initial state steps[0]
        for (int k = 0; k < steps[0].size(); k++) {
            for (int y = 0; y < steps[0][0].size(); y++) {
                for (int x = 0; x < steps[0][0][0].size(); x++) {
                    board[y][x][k] = steps[0][k][y][x];
                }
            }
        }
        // empty the steps vector
        steps.clear();
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
    species.push_back(Species("prey", colors[0]));
    species.push_back(Species("predator", colors[1]));

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
    ImPlot::CreateContext();
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

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
