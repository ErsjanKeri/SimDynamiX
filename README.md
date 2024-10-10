# SymDynamiX in C++

This project is a C++ graphical user interface (GUI) application that allows users to configure and simulate population dynamics on a discrete grid. Users can adjust the grid size, populate it with various species, set interaction coefficients, and observe how populations spread and interact over time according to specified numerical methods.

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
- [Simulation Details](#simulation-details)
    - [Numerical Methods](#numerical-methods)
        - [Population Dispersion](#population-dispersion)
        - [Interaction Coefficients](#interaction-coefficients)
    - [Algorithm Workflow](#algorithm-workflow)
- [Mathematical Formulation of Diffusion](#mathematical-formulation-of-diffusion)
    - [The Diffusion Equation](#the-diffusion-equation)
    - [Laplacian and Spatial Discretization](#laplacian-and-spatial-discretization)
    - [Discretizing the Time Derivative](#discretizing-the-time-derivative)
    - [Combining Time and Space Discretization](#combining-time-and-space-discretization)
    - [Connection to the Code](#connection-to-the-code)
- [Code Structure](#code-structure)
    - [Main Components](#main-components)
    - [Important Variables](#important-variables)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)
- [Additional Information](#additional-information)
    - [Dependencies](#dependencies)
    - [Building from Source](#building-from-source)
    - [Extending the Application](#extending-the-application)
    - [Troubleshooting](#troubleshooting)
- [How the Simulation Works](#how-the-simulation-works)
    - [Population Interaction](#population-interaction)
    - [Population Dispersion](#population-dispersion-1)
    - [Overall Algorithm](#overall-algorithm)
    - [Visualization with ImPlot](#visualization-with-implot)
    - [Key Functions in `Numerical.cpp`](#key-functions-in-numericalcpp)
    - [Understanding the Code](#understanding-the-code)
    - [Conclusion](#conclusion)

---

## Overview

This application simulates the spread and interaction of multiple species across a discrete grid (board). Users can:

- Adjust the dimensions of the grid.
- Add or remove species.
- Configure how species interact with each other using coefficients.
- Set dispersion rates for species.
- Populate the grid with initial populations.
- Run simulations over a specified number of time steps.
- Visualize the population dynamics over time.

## Features

- **Interactive GUI**: Built using Dear ImGui, providing an intuitive interface.
- **Dynamic Configuration**: Adjust grid size and species parameters on the fly.
- **Visualization**: Visual representation of the grid and population distributions.
- **Customizable Interactions**: Define how species affect each other.
- **Simulation Control**: Set the number of time steps and simulate population dynamics.
- **Numerical Methods**: Implements specific numerical algorithms for population dispersion and interaction.

## Getting Started

### Prerequisites

- **C++ Compiler**: A C++17 compliant compiler (e.g., GCC, Clang, MSVC).
- **Dear ImGui**: The immediate mode GUI library.
- **ImPlot**: For plotting and visualizing data within ImGui.
- **CMake**: For building the project (optional but recommended).

### Installation

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/ErsjanKeri/SimDynamiX.git
   ```

2. **Install Dependencies**:

    - Ensure that Dear ImGui and ImPlot are included in your project. You can add them as submodules or include them in your project directory.
    ```bash
    git submodule update --init --recursive
    ```

3. **Build the Project**:

    ## TODO ANTON 

## Usage

Run the compiled executable:

```bash
./population_gui (TODO ANTON)
```

### Configuring the Board

- **Board Width and Height**: Adjust the dimensions of the grid using the sliders labeled "Board Width" and "Board Height".
- The grid resizes dynamically, and you can see the current size printed in the console.

### Managing Species

- **Add Species**: Click the `+` button to add a new species. Each species is assigned a default name and color.
- **Remove Species**: Click the `-` button to remove the last species. The number of species cannot be less than one.
- **Rename Species**: Enter a new name in the input field next to each species to rename it.

### Setting Interaction Coefficients

- **Interaction Matrix**: The table labeled "How does 'column' species affect 'color' one" allows you to set coefficients that define how each species affects others.
    - **Coefficients**: Enter a floating-point number in each cell to represent the interaction strength.
    - **Dispersion**: Set the dispersion rate for each species in the last column. This value must be between 0 and 0.4. You can imagine: the smaller the dispersion, the less "conductive"/less spread happens 

### Running Simulations

- **Timesteps**: Specify the number of timesteps for the simulation in the "Timesteps" input field. The value must be between 1 and 2000.
- **Simulate**: Click the "Simulate" button to run the simulation with the current settings. The application will transition to the simulation view.

### Visualizing the Board

- **Grid Display**: The grid represents the discrete field where populations exist.
    - Each cell shows the population counts for all species in that location.
- **Select a Cell**: Click on a cell to select it. The population details for that cell will be displayed below the grid.
- **Edit Populations**: When a cell is selected, you can adjust the population counts for each species using the input fields provided.

## Simulation Details

### Numerical Methods

The simulation uses specific numerical methods to model how populations spread and interact over time. These methods include:

#### Population Dispersion

To simulate the dispersion (spread) of populations across the grid, the application uses a discrete approximation of diffusion processes. The dispersion is computed separately in one dimension for rows and columns and then combined to yield the two-dimensional dispersion.

- **1D Dispersion**: For each row and column, the dispersion is calculated using a dispersion matrix. This matrix is tridiagonal with values that simulate the diffusion effect.

  **Dispersion Matrix Generation**:

  ```cpp
  vector<vector<int>> generateDispersionMatrix(int n) {
      auto dispersionMatrix = vector(n, vector(n, 0));
      for (int i = 0; i < n; i++) {
          if (i == 0) {
              dispersionMatrix[i][i] = -2;
              dispersionMatrix[i][i + 1] = 1;
          } else if (i == n - 1) {
              dispersionMatrix[i][i - 1] = 1;
              dispersionMatrix[i][i] = -2;
          } else {
              dispersionMatrix[i][i - 1] = 1;
              dispersionMatrix[i][i] = -2;
              dispersionMatrix[i][i + 1] = 1;
          }
      }
      return dispersionMatrix;
  }
  ```

- **Matrix-Vector Multiplication**: The population vector is multiplied by the dispersion matrix to compute the dispersion effect.

  ```cpp
  vector<int> matrixVectorMultiplication(vector<int> vec, vector<vector<int>> matrix) {
      vector<int> resultVector = vector(vec.size(), 0);
      for (int row = 0; row < matrix.size(); row++) {
          int sum = 0;
          for (int col = 0; col < matrix.size(); col++) {
              sum += vec[col] * matrix[row][col];
          }
          resultVector[row] = sum;
      }
      return resultVector;
  }
  ```

- **Combining Row and Column Dispersion**: After computing the dispersion for rows and columns, the results are combined and scaled by the dispersion coefficient.

  ```cpp
  // Combining the 1D dispersions to yield the 2D dispersion
  for (int row = 0; row < population.size(); row++) {
      for (int col = 0; col < population[0].size(); col++) {
          newPopulation[row][col] = static_cast<int>((newPopulationRows[row][col] + newPopulationCols[col][row]) * dispersionCoefficient);
      }
  }
  ```

#### Interaction Coefficients

The interaction between species is modeled using a set of coefficients that define how one species affects another. This is computed using a scalar product of the populations and the coefficients.

- **Scalar Product Calculation**:

  ```cpp
  float scalarProduct(vector<int> & animals, vector<float> & coefficients) {
      float res = 0;
      for (int i = 0; i < animals.size(); i++) {
          res += static_cast<float>(animals[i]) * coefficients[i];
      }
      return res;
  }
  ```

- **Updating Populations**: The population change at each cell is computed based on the interactions.

  ```cpp
  void computeChangedPopulation(vector<vector<vector<int>>> & board, vector<vector<float>> & coefficients) {
      for (int y = 0; y < board.size(); y++) {
          for (int x = 0; x < board[0].size(); x++) {
              for (int k = 0; k < board[0][0].size(); k++) {
                  board[y][x][k] += static_cast<int>(scalarProduct(board[y][x], coefficients[k]));
              }
          }
      }
  }
  ```

### Algorithm Workflow

1. **Initialization**: Populations are initialized on the grid based on user input.
2. **Simulation Loop**: For each timestep:
    - **Population Interaction**: Update populations based on interaction coefficients.
    - **Population Dispersion**: Compute dispersion for each species.
    - **Update Grid**: Apply the computed changes to the grid.
    - **Record State**: Save the current state for visualization.
3. **Visualization**: Display the population distributions over time using heatmaps.

**Main Simulation Function**:

```cpp
void prepareCalculations() {
    add_to_steps(); // Record initial state
    for (int i = 0; i < number_steps_t; i++) {
        computeChangedPopulation(board, coefficients);
        computePopulationsDispersion(board, dispersion_coefficients);
        add_to_steps(); // Record state after each timestep
    }
}
```

# Mathematical Formulation of Diffusion

## The Diffusion Equation

The **diffusion equation** (also known as the **heat equation**) describes how a quantity, such as population density or temperature, diffuses over time:

$$
\frac{\partial u}{\partial t} = D \nabla^2 u
$$


where:

- $`u = u(x, y, t)`$: The quantity being diffused (e.g., population density).
- $`\frac{\partial u}{\partial t}`$: The **time derivative** of $`u`$, representing how $`u`$ changes over time.
- $`D`$: The **diffusion coefficient**, which controls the rate of diffusion.
- $`\nabla^2 u`$: The **Laplacian** of $`u`$, which represents the rate of change in $`u`$ across space, in our case space is only two dimensions (that being width and height of the board).

## Laplacian and Spatial Discretization

The **Laplacian** in two dimensions (for our board) would be:

$$
\nabla^2 u = \frac{\partial^2 u}{\partial x^2} + \frac{\partial^2 u}{\partial y^2}
$$

To approximate this on a **discrete grid**, we use the **finite difference method**.

- $`u_{i,j}`$: The value at grid point ```board[j][i]```.
- Grid spacing in both directions is $`h`$, in our case $`h`$ is implicitly 1.

The **finite difference approximation** of the Laplacian at point $`(i, j)`$  is:

$$
\nabla^2 u_{i,j} \approx \frac{u_{i+1,j} - 2u_{i,j} + u_{i-1,j}}{h^2} + \frac{u_{i,j+1} - 2u_{i,j} + u_{i,j-1}}{h^2}
$$

This formula approximates the second derivatives in the $`x`$ and $`y`$ directions using neighboring points.
How we come to this approximation can be calculated via the Taylor series. 
## Discretizing the Time Derivative

The **time derivative** is also approximated using a **finite difference**. Let:

- $`u_{i,j}^n`$ be the value at grid point $`(i, j)`$ at time step $`n`$.
- Time step is $`\Delta t`$. In our case our timestep is also implicitly 1, as our initial goal was to work with discrete values. 

Using the **explicit Euler method**:

$$
\frac{\partial u}{\partial t} \approx \frac{u_{i,j}^{n+1} - u_{i,j}^n}{\Delta t}
$$

## Combining Time and Space Discretization

By combining the discretized time and space derivatives, we approximate the diffusion equation as:

$$
\frac{u_{i,j}^{n+1} - u_{i,j}^n}{\Delta t} = D \left( \frac{u_{i+1,j}^n - 2u_{i,j}^n + u_{i-1,j}^n}{h^2} + \frac{u_{i,j+1}^n - 2u_{i,j}^n + u_{i,j-1}^n}{h^2} \right)
$$

Rearranging to solve for $`u_{i,j}^{n+1}`$:

$$
u_{i,j}^{n+1} = u_{i,j}^n + \Delta t \cdot D \left( \frac{u_{i+1,j}^n - 2u_{i,j}^n + u_{i-1,j}^n}{h^2} + \frac{u_{i,j+1}^n - 2u_{i,j}^n + u_{i,j-1}^n}{h^2} \right)
$$

This equation tells us how to update the value of $`u`$ at each grid point from time step $`n`$ to $`n+1`$, displaying the effects of diffusion.
This last step is essentially what happens in ```computePopulationsDispersion(vector<vector<vector<int>>> & populations, vector<float> dispersionCoefficients)```
## Connection to the Code

- The **dispersion matrix** in the code acts as a discrete Laplacian in 1D, imagine the head equation on a rod, there you can see where does the ```-1, 2, -1``` pattern come from.
- The **finite difference** scheme is used to compute how each point’s value changes based on its neighbors. It is applied once for all rows and once for all columns. 
- This is equivalent to applying the **2D Laplacian**, calculated separately for rows and columns, to approximate the **diffusion** of populations over time.

In summary, the code aims to approximate the **heat equation** using **finite difference methods** to compute both the **spatial Laplacian** and the **temporal derivative**, thereby modeling how populations disperse across the grid over time.


## Code Structure

### Main Components

- **`config_board_size_species()`**:
    - Handles the configuration of the board size and the number of species.
    - Updates the grid dimensions and manages species addition and removal.
- **`config_species_list()`**:
    - Provides input fields to rename species.
- **`config_dynamics()`**:
    - Displays the interaction matrix where users set coefficients for species interactions.
    - Includes input for dispersion coefficients.
    - Contains the "Simulate" button to start the simulation.
- **`board_render()`**:
    - Renders the grid where populations are visualized.
    - Allows users to select cells and adjust populations.
    - Displays population details for the selected cell.
- **`Numerical.h` and `Numerical.cpp`**:
    - Contain the numerical methods used for simulating population dispersion and interaction.
    - Key functions include `computePopulationsDispersion`, `computeChangedPopulation`, and `prepareCalculations`.

### Important Variables

- **`board`**: A 3D vector representing the grid. Each cell contains a vector of population counts for each species.
- **`species`**: A vector of `Species` objects representing each species in the simulation.
- **`coefficients`**: A 2D vector holding interaction coefficients between species.
- **`dispersion_coefficients`**: A vector containing dispersion rates for each species.
- **`number_steps_t`**: An integer representing the total number of timesteps for the simulation.
- **`selected_box`**: An integer representing the currently selected cell in the grid.
- **`steps`**: A vector that records the state of the grid at each timestep for visualization. Basically an array of ```board```

## Contributing

If you would like to improve accuracy or efficiency, contributions are welcomed! Please open issues or pull requests for any changes or additions you'd like to make.

1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/YourFeature`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin feature/YourFeature`).
5. Open a pull request.

## Used Libraries

- [Dear ImGui](https://github.com/ocornut/imgui): For the immediate mode GUI framework.
- [ImPlot](https://github.com/epezent/implot): For advanced plotting capabilities within ImGui.
- [C++ Standard Library](https://en.cppreference.com/w/): For data structures and algorithms.

---

## Additional Information

### Dependencies

- **ImGui**: The core GUI library used to build the interface.
- **ImPlot**: An extension to ImGui that provides advanced plotting features.
- **SDL2**: Used for window creation and input handling.
- **OpenGL**: For rendering graphics.

### Building from Source

Ensure all dependencies are correctly installed and linked. Adjust your build system to include paths to ImGui and ImPlot headers and link against necessary libraries.

#### Example Build Command

```bash
TODO ANTON 
```

