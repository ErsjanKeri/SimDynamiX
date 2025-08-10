#include "imgui.h"
#include "implot.h"
#include "./settings.h"
#include "vector"

// TODO labels should be generated only once
// TODO add a text for every graph with the name of species
// TODO fix by calculation at numerical to prevent reconversion to correct format
// TODO pop dynamics dp/dt = k*q when p is but 0 should not change
// TODO optimisation possible through SIMD
// TODO either let the user input themselves the scaling_max or calculate it similar to median
using namespace std;

// index = selected_timestep-1
int selected_timestep = 1;


void generate_labels(vector<std::string>& labels, int count, const char* prefix) {
    labels.clear();
    for (int i = 0; i < count; ++i) {
        labels.push_back(std::string(prefix) + std::to_string(i + 1)); // Label names like "R1", "R2", ...
    }
}

static bool auto_scale = true;

void simulations_render_header() {
    const float avail_w = ImGui::GetContentRegionAvail().x;
    double size_x = std::max(40.0f, avail_w / 3.0f);

    ImGui::SetNextItemWidth(size_x-90);
    static ImPlotColormap map = ImPlotColormap_Viridis;
    if (ImPlot::ColormapButton(ImPlot::GetColormapName(map),ImVec2(size_x-90,0),map)) {
        map = (map + 1) % ImPlot::GetColormapCount();
        ImPlot::BustColorCache("##Heatmap1");
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(size_x-150);
    ImGui::LabelText("##Colormap Index", "%s", "Change Heatmap");
    ImGui::SliderInt("Timestep", &selected_timestep, 1, number_steps_t, "%d", ImGuiSliderFlags_NoInput);
    ImGui::Checkbox("Auto scale heatmaps", &auto_scale);
    if (compare_methods) ImGui::Text("Left: Current method   Right: Other method");
}

void simulations_render_grids() {
    // Compute per-grid width to avoid horizontal overflow.
    // Each table column hosts one heatmap plus a ~50px colorbar drawn on the same line.
    const float colorbar_w = 50.0f;
    const int cols = 2; // at most 2 grids per row always
    const float avail_w = ImGui::GetContentRegionAvail().x;
    const float inter_group_spacing = 12.0f; // padding inside cell (approx)
    double size_x = std::max(40.0f, (avail_w - cols * (colorbar_w + inter_group_spacing)) / cols);

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 originalCellPadding = style.CellPadding;     // Save original cell padding
    style.CellPadding = ImVec2(10, 10);
    static ImPlotHeatmapFlags hm_flags = 0; static ImPlotColormap map = ImPlotColormap_Viridis;
    if (ImGui::BeginTable("Grid Table", cols, ImGuiTableFlags_SizingFixedFit )) {
        auto draw_one = [&](int species_index, const std::vector<std::vector<std::vector<std::vector<double>>>>& src_steps, const char* tag){
            int nrows = board_height, ncols = board_width;
            static std::vector<double> flat; flat.resize(nrows*ncols);
            for (int y=0;y<nrows;++y) for (int x=0;x<ncols;++x) flat[y*ncols+x]=src_steps[selected_timestep-1][species_index][y][x];
            // labels
            vector<std::string> xlabels_str; vector<const char*> xlabels_cstr;
            vector<std::string> ylabels_str; vector<const char*> ylabels_cstr;
            generate_labels(xlabels_str, board_width, "C"); generate_labels(ylabels_str, board_height, "R");
            xlabels_cstr.reserve(xlabels_str.size()); for (const auto& s:xlabels_str) xlabels_cstr.push_back(s.c_str());
            ylabels_cstr.reserve(ylabels_str.size()); for (const auto& s:ylabels_str) ylabels_cstr.push_back(s.c_str());
            static ImPlotAxisFlags axes_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;
            // autoscale
            static double scale_min = 0.0; double scale_max = 100.0;
            if (auto_scale) { double mx=0.0; for (double v: flat) mx = std::max(mx, v); scale_max = std::max(1.0, mx); }
            std::string tag_str = std::string(tag);
            if (ImPlot::BeginPlot((std::string("##Heatmap")+tag_str+std::to_string(species_index)).c_str(), ImVec2(size_x,size_x), ImPlotFlags_NoLegend|ImPlotFlags_NoMouseText)){
                ImPlot::SetupAxes(nullptr, nullptr, axes_flags, axes_flags);
                ImPlot::SetupAxisTicks(ImAxis_X1, 0 + 1.0/(2*board_width), 1 - 1.0/(2*board_width), board_width, xlabels_cstr.data());
                ImPlot::SetupAxisTicks(ImAxis_Y1, 1 - 1.0/(2*board_height), 0 + 1.0/(2*board_height), board_height, ylabels_cstr.data());
                ImPlot::PlotHeatmap((std::string("heat")+tag_str+std::to_string(species_index)).c_str(), flat.data(), board_height, board_width, scale_min, scale_max, nullptr, ImPlotPoint(0,0), ImPlotPoint(1,1), hm_flags);
                ImPlot::EndPlot();
            }
            ImGui::SameLine(); ImPlot::ColormapScale((std::string("##HeatScale")+tag_str+std::to_string(species_index)).c_str(), scale_min, scale_max, ImVec2(50,size_x));
        };

        if (!compare_methods) {
            for (int i=0; i<species.size(); ) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImPlot::PushColormap(map); draw_one(i, steps, "A"); ImPlot::PopColormap(); ++i;
                if (i<species.size()) { ImGui::TableSetColumnIndex(1); ImPlot::PushColormap(map); draw_one(i, steps, "B"); ImPlot::PopColormap(); ++i; }
            }
        } else {
            const auto& ref_steps = (diffusion_method == DIFFUSION_ADI && steps_explicit.size()) ? steps_explicit : (steps_adi.size() ? steps_adi : steps);
            for (int i=0; i<species.size(); ++i) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImPlot::PushColormap(map); draw_one(i, steps, "L"); ImPlot::PopColormap();
                ImGui::TableSetColumnIndex(1); ImPlot::PushColormap(map); draw_one(i, ref_steps, "R"); ImPlot::PopColormap();
            }
        }
        ImGui::EndTable(); ImGui::Spacing();
    }
    style.CellPadding = originalCellPadding;
}