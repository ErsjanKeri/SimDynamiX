cmake_minimum_required(VERSION 3.15)
project(SimDynamiX)

set(CMAKE_CXX_STANDARD 20)
# Add source files
set(SOURCES
        main.cpp
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_widgets.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/misc/cpp/imgui_stdlib.cpp
        third_party/imgui/backends/imgui_impl_glfw.cpp
        third_party/imgui/backends/imgui_impl_opengl3.cpp
        third_party/implot/implot.cpp
        third_party/glad/src/glad.c
        third_party/Species.cpp
        third_party/Species.h
        third_party/Numerical.cpp
        third_party/implot/implot_items.cpp
        third_party/Configuration.cpp
        third_party/Presets.cpp
        third_party/Presets.h
        third_party/Simulation.h
        third_party/Simulation.cpp
)

# Find GLFW and OpenGL
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Include directories for IMGUI
include_directories(third_party/imgui)
include_directories(third_party/imgui/backends)

# Include directories for implot
include_directories(third_party/implot)

# Include directories for GLAD and GLFW
include_directories(third_party/glad/include)  # Include GLAD headers
include_directories(third_party/glad/include/KHR)      # Include KHR headers
include_directories(${GLFW_INCLUDE_DIRS})

# Link libraries
add_executable(SimDynamiX ${SOURCES}
        third_party/Species.cpp
        third_party/Species.h)
target_link_libraries(SimDynamiX OpenGL::GL glfw)

