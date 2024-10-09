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

#include "third_party/Species.h"
#include "third_party/settings.h"

using namespace std;

// TODO, between 1 - 20 tiles
// f(x,y,t) is actually density function at t
// TODO, coefficient of reproduction



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

    int slice = 0;

    // Flatten the 2D slice (board of size [board_height][board_width][15]) into 1D data for heatmap visualization
    vector<float> flattened_data;
    for (int i = 0; i < board.size(); ++i) {
        for (int j = 0; j < board[i].size(); ++j) {
            flattened_data.push_back(static_cast<float>(board[i][j][slice]));
        }
    }


    ImGui::Begin("Rendering Simulation", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    //     if (ImPlot::BeginPlot("2D Scalar Field")) {
    //         // Customize the heatmap parameters as needed
    //         ImPlot::PlotHeatmap<float>(
    //     "ScalarField",
    //             flattened_data.data(),
    //             board.size(),
    //             board[0].size(),
    //             0,
    //             10,
    //             nullptr,
    //             ImPlotPoint(0, 0),
    //             ImPlotPoint(board[0].size(),board.size())
    //         );
    //         ImPlot::EndPlot();
    //     }
    // ImGui::End();
    // TODO, implement this for our data
    static float values1[7][7]  = {{0.8f, 2.4f, 2.5f, 3.9f, 0.0f, 4.0f, 0.0f},
                                    {2.4f, 0.0f, 4.0f, 1.0f, 2.7f, 0.0f, 0.0f},
                                    {1.1f, 2.4f, 0.8f, 4.3f, 1.9f, 4.4f, 0.0f},
                                    {0.6f, 0.0f, 0.3f, 0.0f, 3.1f, 0.0f, 0.0f},
                                    {0.7f, 1.7f, 0.6f, 2.6f, 2.2f, 6.2f, 0.0f},
                                    {1.3f, 1.2f, 0.0f, 0.0f, 0.0f, 3.2f, 5.1f},
                                    {0.1f, 2.0f, 0.0f, 1.4f, 0.0f, 1.9f, 6.3f}};
    static float scale_min       = 0;
    static float scale_max       = 6.3f;
    static const char* xlabels[] = {"C1","C2","C3","C4","C5","C6","C7"};
    static const char* ylabels[] = {"R1","R2","R3","R4","R5","R6","R7"};

    static ImPlotColormap map = ImPlotColormap_Viridis;
    if (ImPlot::ColormapButton(ImPlot::GetColormapName(map),ImVec2(225,0),map)) {
        map = (map + 1) % ImPlot::GetColormapCount();
        ImPlot::BustColorCache("##Heatmap1");
    }
    ImGui::SameLine();
    ImGui::LabelText("##Colormap Index", "%s", "Change Colormap");
    ImGui::SetNextItemWidth(225);
    ImGui::DragFloatRange2("Min / Max",&scale_min, &scale_max, 0.01f, -20, 20);

    static ImPlotHeatmapFlags hm_flags = 0;

    ImGui::CheckboxFlags("Column Major", (unsigned int*)&hm_flags, ImPlotHeatmapFlags_ColMajor);

    static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

    ImPlot::PushColormap(map);

    if (ImPlot::BeginPlot("##Heatmap1",ImVec2(225,225),ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)) {
        ImPlot::SetupAxes(nullptr, nullptr, axes_flags, axes_flags);
        ImPlot::SetupAxisTicks(ImAxis_X1,0 + 1.0/14.0, 1 - 1.0/14.0, 7, xlabels);
        ImPlot::SetupAxisTicks(ImAxis_Y1,1 - 1.0/14.0, 0 + 1.0/14.0, 7, ylabels);
        ImPlot::PlotHeatmap("heat",values1[0],7,7,scale_min,scale_max,"%g",ImPlotPoint(0,0),ImPlotPoint(1,1),hm_flags);
        ImPlot::EndPlot();
    }
    ImGui::SameLine();
    ImPlot::ColormapScale("##HeatScale",scale_min, scale_max, ImVec2(60,225));

    ImGui::End();

    if (ImGui::Button("Stop Simulation")) {
        current = CONFIGURATION;
        // done = true;
    }
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
