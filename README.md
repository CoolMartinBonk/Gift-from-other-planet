# Gift From Other Planet 

> **Dive into a living alien world found on your screen.**

**Gift From Other Planet** is an interactive generative art application built from scratch with **C++** and **SDL2**. It combines physics-based fluid simulation with real-time procedural audio synthesis to create a mesmerizing audio-visual experience.

Every movement creates its own unique sound. Touch the fluid, listen to the math, and explore a symphony of light and chaos.

---

##  Features

*   **Interactive Fluid Dynamics**: A custom SPH (Smoothed-Particle Hydrodynamics) system simulating thousands of particles.
*   **Generative Audio Engine**: Real-time sound synthesis using FM modulation and Schroeder reverb algorithms. No pre-recorded sound effects—the physics generates the audio.
*   **Metaball Rendering**: Organic, glowing visual style that merges particles into liquid forms.
*   **Creative Sandbox**: Paint with rainbow particles or trigger explosions.
*   **Cross-Platform**: Built with CMake for easy deployment on Windows, Linux, and macOS.

---

##  Controls

| Input | Action |
| :--- | :--- |
| **Left Mouse (Hold)** | Repel particles / Paint (in Brush Mode) |
| **Right Mouse** | Toggle **Brush Mode** (Hides cursor) |
| **Key '1'** | **Blue Brush** (Standard liquid) |
| **Key '2'** | **Rainbow Brush** (Color-shifting particles) |
| **Key 'F3'** | Show/Hide FPS Counter |

---

##  Build Instructions

This project uses **CMake** to manage the build process.

### Prerequisites

*   **C++ Compiler** (GCC, Clang, or MSVC)
*   **CMake** (3.10 or newer)

###  Windows

*Note: SDL2 libraries are included in the `libs/` folder, so no external installation is required.*

1. Open a terminal (CMD or PowerShell) in the project root.
2. Run the following commands:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be generated in the build/Release/ folder.

###  Unix-like Environments (Linux & macOS)
For Linux distributions and macOS, a unified build process is supported.

1. Install Dependencies

<details> <summary><strong>Click to expand installation commands</strong></summary>

Platform	Command
Ubuntu/Debian	sudo apt install build-essential cmake libsdl2-dev
Arch Linux	sudo pacman -S base-devel cmake sdl2
Fedora	sudo dnf install cmake gcc-c++ SDL2-devel
macOS (Homebrew)	brew install cmake sdl2
</details>

2. Build & Run
```Bash
# Configure and Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run
./build/GiftFromOtherPlanet

``` 




