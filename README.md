# SymDynamiX in C++

This project is a C++ graphical user interface (GUI) application that allows users to configure and simulate population dynamics on a discrete grid. Users can adjust the grid size, populate it with various species, set interaction coefficients, and observe how populations spread and interact over time according to selected numerical methods.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
  - [Configuring the Board](#configuring-the-board)
  - [Managing Species](#managing-species)
  - [Setting Interaction Coefficients](#setting-interaction-coefficients)
  - [Running Simulations](#running-simulations)
  - [Visualizing the Board](#visualizing-the-board)
  - [Preset Scenarios](#preset-scenarios)
- [Simulation Details](#simulation-details)
  - [Numerical Methods](#numerical-methods)
    - [Population Interaction](#population-interaction)
    - [Population Dispersion](#population-dispersion)
    - [Algorithm Workflow](#algorithm-workflow)
- [Mathematical Formulation of Diffusion](#mathematical-formulation-of-diffusion)
  - [The Diffusion Equation](#the-diffusion-equation)
  - [Laplacian and Spatial Discretization](#laplacian-and-spatial-discretization)
  - [Time Discretization](#time-discretization)
  - [Explicit vs. ADI](#explicit-vs-adi)
  - [Boundary Conditions](#boundary-conditions)
- [Code Structure](#code-structure)
  - [Main Components](#main-components)
  - [Important Variables](#important-variables)
- [Contributing](#contributing)
- [Used Libraries](#used-libraries)
- [Additional Information](#additional-information)
  - [Dependencies](#dependencies)
  - [Building from Source](#building-from-source)
  - [Troubleshooting](#troubleshooting)

---

## Overview

This application simulates the spread and interaction of multiple species across a discrete grid (board). Users can:

- Adjust the dimensions of the grid.
- Add or remove species.
- Configure how species interact with each other using coefficients.
- Set dispersion rates for species.
- Choose a diffusion method (Explicit vs. ADI) and boundary conditions.
- Control the simulation time step.
- Compare diffusion methods side-by-side.
- Visualize the population dynamics over time.

## Features

- **Interactive GUI**: Built with Dear ImGui for immediate-mode UI.
- **Dynamic Configuration**: Adjust grid size and species parameters on the fly.
- **Multiple Numerical Methods**: Choose between an explicit finite-difference scheme and an ADI (Crank–Nicolson) scheme.
- **Boundary Conditions**: Select Dirichlet (absorbing) or Neumann (zero-flux).
- **Time-Step Control**: Set the simulation time step to explore stability and convergence behavior.
- **Method Comparison**: Optional side-by-side visualization comparing the current method with the alternative.
- **Visualization**: Heatmaps with selectable colormaps and autoscaling.

## Getting Started

### Prerequisites

- **C++ Compiler**: C++17+ (e.g., GCC, Clang, MSVC).
- **CMake**: For building the project.
- **GLFW + OpenGL**: Windowing and graphics.
- **Dear ImGui + ImPlot**: GUI and plotting.

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/ErsjanKeri/SimDynamiX.git
   ```
2. Initialize submodules (if applicable):
   ```bash
   git submodule update --init --recursive
   ```
3. Build:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## Usage

Run the executable:
```bash
./SimDynamiX
```

### Configuring the Board

- Set **Board Width** and **Board Height** with sliders (grid resizes immediately).

### Managing Species

- **Add Species**: Click “+”.
- **Remove Species**: Click “-”.
- **Rename Species**: Edit the name in the species list.

### Setting Interaction Coefficients

- In “Dynamics Between Species/Cells”, fill the interaction matrix:
  - Each table cell sets how “column species” affects the “row/color species”.
  - Set per-species **Dispersion** in the last column (clamped to [0, 0.4]).

- Choose numeric options:
  - **Diffusion Method**: Explicit (FD) or ADI (Crank–Nicolson).
  - **Boundary Condition**: Dirichlet (absorbing) or Neumann (zero-flux).
  - **Delta t**: Simulation time step.
  - Optional: **Compare Explicit vs ADI (side-by-side)** toggles a dual-panel view in the Simulation.

- A hint is shown if explicit settings may be unstable (rule of thumb for uniform grid spacing: max D × Δt ≲ 0.25).

### Running Simulations

- **Timesteps**: Enter the number of steps to run (1–2000).
- Click **Simulate** to run and switch to the simulation view.
- Click **Stop Simulation** to return to the configuration view (restores the initial state of the current run).

### Visualizing the Board

- The Simulation view shows per-species heatmaps.
- Use the **Timestep** slider to scrub through time.
- Change **colormap** from the toolbar.
- Enable **Auto scale heatmaps** to adapt color range to data.
- When comparison is enabled, left shows the current method and right shows the alternate method for the same configuration.

### Preset Scenarios

The app ships with a small gallery of curated presets (inspired by classic pattern‑formation demos and research visuals found on the internet). They are designed to be visually engaging out‑of‑the‑box and to highlight how parameters, boundary conditions and numerical methods affect the dynamics.

- Access them from the Presets section in the Dynamics panel.
- Click Load Preset to populate the board, set the method, boundary condition, diffusion rates, time step and number of steps.
- Presets are board‑size aware: initial shapes and radii scale with the current grid; intensities are chosen so patterns are visible immediately.

Summary of included presets:

| Preset | Species | Method | Boundary | Δt | Steps | Visual motif |
|---|---|---|---|---:|---:|---|
| Two‑Front Collision | 1 | Explicit (FD) | Dirichlet | 0.8 | 200 | Two dense fronts invade from left and right, colliding near the center |
| Predator Core vs Prey Ring | 2 (prey, predator) | ADI (Crank–Nicolson) | Neumann | 1.0 | 220 | Predator core seeded inside a prey ring; chasing waves form as predator diffuses faster |
| Cyclic Triad Waves | 3 (R,P,S) | ADI | Neumann | 1.0 | 240 | Rock–paper–scissors‑like cyclic dominance, three seeds at triangle vertices produce traveling wavelets |
| Competitive Domains From Noise | 2 (blue, yellow) | Explicit | Neumann | 0.7 | 300 | Weakly antagonistic competitors segregate into domains starting from small random noise |
| Four Corners Convergence | 4 (A,B,C,D) | Explicit | Dirichlet | 0.6 | 180 | Four species seeded at corners diffuse and interact toward the center |
| Fast vs Slow Diffusion Twins | 2 (slow, fast) | ADI | Neumann | 1.0 | 200 | Identical initial blobs with very different diffusion; clear speed contrast |

Notes and tips:

- The presets intentionally use moderate to large time steps for fast feedback; ADI is robust at larger Δt, while Explicit benefits from Δt × max(D) ≲ 0.25.
- Use side‑by‑side comparison to immediately see method differences; the app renders each species’ pair next to each other.
- You can still adjust coefficients or dispersion after loading a preset; the next simulation run will use your edits.

## Simulation Details

### Numerical Methods

#### Population Interaction
- At each cell, the species vector is updated from a linear interaction model via a coefficient matrix (one set of coefficients per “affected” species).

#### Population Dispersion
- Diffusion is computed per species on the grid.
- Two methods are available:
  - **Explicit finite-difference**: Applies a discrete Laplacian scaled by the diffusion coefficient and time step.
  - **ADI (Crank–Nicolson)**: Alternating-direction implicit method that is unconditionally stable for linear diffusion and supports both Dirichlet and Neumann boundary conditions.

#### Algorithm Workflow

For each timestep:
1. Update populations using the interaction coefficients (per cell, per species).
2. Apply diffusion using the selected numerical method and boundary condition.
3. Record the state for visualization.

## Mathematical Formulation of Diffusion

### The Diffusion Equation

The **diffusion equation** (also known as the **heat equation**) describes how a quantity diffuses over time:

$$
\frac{\partial u}{\partial t} = D \nabla^2 u
$$

where:

- $`u = u(x, y, t)`$: the diffused quantity (e.g., population density). ```steps[t][y][x]``` $`\approx u(x,y,t)`$.
- $`D`$: the diffusion coefficient controlling the rate of diffusion.
- $`\nabla^2 u`$: the Laplacian of $`u`$.

### Laplacian and Spatial Discretization

On a 2D grid with unit spacing $`h=1`$ the Laplacian is approximated by the standard 5‑point stencil:

$$
\nabla^2 u_{i,j} \approx \frac{u_{i+1,j} - 2u_{i,j} + u_{i-1,j}}{h^2} + \frac{u_{i,j+1} - 2u_{i,j} + u_{i,j-1}}{h^2}
$$

### Time Discretization

Using the **explicit Euler** method:

$$
u_{i,j}^{n+1} = u_{i,j}^{n} + \Delta t\, \frac{\partial u}{\partial t} \quad \Rightarrow \quad \frac{\partial u}{\partial t} \approx \frac{u_{i,j}^{n+1} - u_{i,j}^{n}}{\Delta t}
$$

Combining time and space discretizations gives the explicit update:

$$
\frac{u_{i,j}^{n+1} - u_{i,j}^{n}}{\Delta t} = D\left( \frac{u_{i+1,j}^{n} - 2u_{i,j}^{n} + u_{i-1,j}^{n}}{h^2} + \frac{u_{i,j+1}^{n} - 2u_{i,j}^{n} + u_{i,j-1}^{n}}{h^2} \right)
$$

or equivalently

$$
u_{i,j}^{n+1} = u_{i,j}^{n} + \Delta t\, D\left( \frac{u_{i+1,j}^{n} - 2u_{i,j}^{n} + u_{i-1,j}^{n}}{h^2} + \frac{u_{i,j+1}^{n} - 2u_{i,j}^{n} + u_{i,j-1}^{n}}{h^2} \right)
$$

For explicit schemes on a unit grid, a common stability guideline is $`D\,\Delta t \lesssim 0.25`$.

The ADI (Crank–Nicolson) scheme updates by alternating implicit 1D solves in x and y and supports both Dirichlet (absorbing) and Neumann (zero‑flux) boundary conditions.

## Code Structure

### Main Components

- `config_board_size_species()`:
  - Configures board size and the number of species.
- `config_species_list()`:
  - Lets you rename species.
- `config_dynamics()`:
  - Interaction matrix, dispersion inputs, diffusion method selection, boundary condition selection, delta t, timesteps, and simulate button.
  - Optional method comparison toggle.
- `board_render()`:
  - Renders the grid for editing initial populations.
- `simulations_list()`:
  - Displays per-species heatmaps over time (with optional side-by-side comparison).
- `Numerical.h/.cpp`:
  - Numerical routines for interaction and diffusion (supports explicit and ADI).

### Important Variables

- `board`: 3D grid `[y][x][species]` for current populations.
- `species`: List of species with display names and colors.
- `coefficients`: Interaction matrix (per “affected” species row).
- `dispersion_coefficients`: Per-species diffusion rates.
- `delta_time`: Time step used by the numerical schemes.
- `boundary_condition`: Dirichlet or Neumann.
- `diffusion_method`: Explicit or ADI.
- `number_steps_t`: Number of timesteps per run.
- `selected_box`: Currently selected cell for editing.
- `steps`: Recorded states `[step][species][y][x]` for visualization.
- When comparing methods, `steps_explicit` and `steps_adi` record alternative runs for the same setup.

## Contributing

Contributions are welcome! Please open issues or pull requests for improvements, features, or documentation updates.

1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/YourFeature`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin feature/YourFeature`).
5. Open a pull request.

## Used Libraries

- [Dear ImGui](https://github.com/ocornut/imgui)
- [ImPlot](https://github.com/epezent/implot)
- [GLFW](https://www.glfw.org/)
- [OpenGL](https://www.khronos.org/opengl/)

---

## Additional Information

### Dependencies

- Dear ImGui (GLFW + OpenGL3 backend)
- ImPlot
- GLFW
- OpenGL 4.1 (Core) context

### Building from Source

See [Installation](#installation). Ensure your system provides a recent C++ compiler and OpenGL/GLFW development packages.

### Troubleshooting

- If the window doesn’t open or crashes at startup, validate your OpenGL driver supports 4.1 Core.
- For rendering issues, try different ImPlot colormaps or toggle the autoscale option.
- If explicit runs behave erratically, consider reducing diffusion or time step, or switch to ADI.
