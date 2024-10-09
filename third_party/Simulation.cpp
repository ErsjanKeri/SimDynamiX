#include "imgui.h"
#include "implot.h"
#include "./settings.h"
#include "vector"



using namespace std;

// index = selected_timestep-1
int selected_timestep = 1;


void generate_labels(vector<std::string>& labels, int count, const char* prefix) {
    labels.clear();
    for (int i = 0; i < count; ++i) {
        labels.push_back(std::string(prefix) + std::to_string(i + 1)); // Label names like "R1", "R2", ...
    }
}

void simulations_list() {
    double size_x = (CANVAS_WIDTH-2*PADDING)/2 -80;

    ImGui::SetNextItemWidth(size_x-90);
    static ImPlotColormap map = ImPlotColormap_Viridis;
    if (ImPlot::ColormapButton(ImPlot::GetColormapName(map),ImVec2(size_x-90,0),map)) {
        map = (map + 1) % ImPlot::GetColormapCount();
        ImPlot::BustColorCache("##Heatmap1");
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(size_x-150);
    ImGui::LabelText("##Colormap Index", "%s", "Change Heatmap");
    static ImPlotHeatmapFlags hm_flags = 0;

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 originalCellPadding = style.CellPadding;     // Save original cell padding
    style.CellPadding = ImVec2(10, 10);

    // slider to change value of selected_timestep, possible values from 1 to number_steps_t
    ImGui::SliderInt("Timestep", &selected_timestep, 1, number_steps_t);

    if (ImGui::BeginTable("Grid Table", 3, ImGuiTableFlags_SizingFixedFit )) {

        for (int row = 0; row < (species.size()+2)/3; row++) {
            ImGui::TableNextRow();  // Move to the next row
            for (int col = 0; col < min(3, static_cast<int>(species.size() - 3 * row)); col++) {
                int i = row*3+col;
                int nrows = board_height;
                int ncols = board_width;

                int* flat_values = new int[nrows * ncols];
                for (int y = 0; y < nrows; y++) {
                    for (int x = 0; x < ncols; x++) {
                        flat_values[y * ncols + x] = steps[selected_timestep-1][i][y][x];
                    }
                }


                ImGui::TableSetColumnIndex(col);  // Set the current column

                static float scale_min       = 0;
                static float scale_max       = 100;

                // LABELS
                // Generate labels
                vector<std::string> xlabels_str;
                vector<const char*> xlabels_cstr;
                vector<std::string> ylabels_str;
                vector<const char*> ylabels_cstr;

                generate_labels(xlabels_str, board_width, "C");  // Column labels C1, C2, C3, ...
                generate_labels(ylabels_str, board_height, "R"); // Row labels R1, R2, R3, ...

                // Store c_str() pointers
                xlabels_cstr.reserve(xlabels_str.size());
                for (const auto& s : xlabels_str) {
                    xlabels_cstr.push_back(s.c_str());
                }

                ylabels_cstr.reserve(ylabels_str.size());
                for (const auto& s : ylabels_str) {
                    ylabels_cstr.push_back(s.c_str());
                }
                // END LABELS



                static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

                ImPlot::PushColormap(map);

                if (ImPlot::BeginPlot(("##Heatmap1"+to_string(i)).c_str(),ImVec2(size_x,size_x),ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)) {
                    ImPlot::SetupAxes(nullptr, nullptr, axes_flags, axes_flags);
                    ImPlot::SetupAxisTicks(ImAxis_X1, 0 + 1.0 / (2 * board_width), 1 - 1.0 / (2 * board_width), board_width, xlabels_cstr.data());
                    ImPlot::SetupAxisTicks(ImAxis_Y1, 1 - 1.0 / (2 * board_height), 0 + 1.0 / (2 * board_height), board_height, ylabels_cstr.data());
                    ImPlot::PlotHeatmap(("heat"+to_string(i)).c_str(),flat_values,board_height,board_width,scale_min,scale_max,"%i",ImPlotPoint(0,0),ImPlotPoint(1,1),hm_flags);
                    ImPlot::EndPlot();
                }


                // plot the graph for board[selected_step][i]

                ImGui::SameLine();
                ImPlot::ColormapScale("##HeatScale",scale_min, scale_max, ImVec2(50,size_x));
            }
        }
        ImGui::EndTable();
        ImGui::Spacing();
    }
    style.CellPadding = originalCellPadding;
}