# Real-Time Procedural Audio Synthesizer & Telemetry Bridge (B.Eng. Thesis)

> **Status:** In Progress — Bachelor of Engineering Thesis, Faculty of Technical Physics, Information Technology and Applied Mathematics (FTIMS), Lodz University of Technology.

A real-time procedural audio synthesis system written in **C++**, designed to dynamically generate and modulate creature vocalizations and breathing without relying on pre-recorded static audio assets. The project consists of a native game telemetry plugin (**F4SE**) and a standalone **JUCE** DSP synthesizer communicating over low-latency **UDP/OSC**.

## Architecture Overview

```text
+-------------------+       F4SE Hook        +----------------------+
|    Fallout 4      | ---------------------> |     F4SE_plugin      |
|  (Live Game Data) |   (speed, health,     |  (C++, CMake, vcpkg) |
+-------------------+    scale, combat,      +----------+-----------+
                         actions)                       |
                                                        | UDP / OSC Telemetry
                                                        v
+-------------------+     Real-Time Audio    +----------------------+
|   Audio Output    | <--------------------- |     JUCE_plugin      |
|  (Procedural SFX) |                        |  (C++, JUCE, DSP)    |
+-------------------+                        +----------------------+
```

## Tech Stack

- **Language:** C++ (Modern C++)
- **Audio & DSP:** JUCE Framework, Real-Time Procedural Synthesis
- **Networking / IPC:** UDP, Open Sound Control (OSC)
- **Native Integration:** Fallout 4 Script Extender (F4SE)
- **Build & Dependency Management:** CMake (`CMakePresets.json`), vcpkg (`vcpkg.json`), Projucer

## Key Engineering Highlights

- **Decoupled Two-Process Architecture:** Separates native in-game state extraction (`F4SE_plugin`) from real-time audio processing (`JUCE_plugin`), ensuring audio DSP execution does not block the game loop.
- **Low-Latency Telemetry Streaming:** Streams live entity state parameters (movement speed, health, scale, combat state, attack actions, and damage events) over UDP/OSC.
- **Dynamic DSP Parameter Mapping:** Continuously modulates synthesis parameters in real time to produce reactive breathing, attack vocalizations, damage feedback, and death roars tailored to entity state.
- **Modern C++ Build Pipeline:** Configured with CMake presets, precompiled headers (`PCH.hpp`), and declarative dependency management via `vcpkg.json`.

## Repository Structure

```text
.
├── F4SE_plugin/                # Native C++ telemetry extractor for Fallout 4 (F4SE)
│   ├── CMakeLists.txt          # CMake build configuration
│   ├── CMakePresets.json       # Build presets
│   ├── vcpkg.json              # vcpkg dependency manifest
│   ├── cmake/                  # Version & plugin metadata templates
│   ├── include/                # Shared headers and precompiled header (PCH.hpp)
│   └── src/main.cpp            # Plugin entry point, state polling, and OSC sender
├── JUCE_plugin/                # Real-time procedural audio synthesizer (JUCE)
│   ├── ProofOfConcept.jucer    # Projucer project file
│   └── Source/
│       ├── Main.cpp            # Application bootstrap
│       └── MainComponent.*     # OSC receiver, UI controls, and DSP synthesis engine
└── README.md
```

## Building the Project

### Prerequisites

- C++ compiler with MSVC / Visual Studio 2022 (Windows)
- CMake (with preset support) & `vcpkg`
- JUCE & Projucer

### 1. Native Telemetry Plugin (`F4SE_plugin`)

Configure and build using the presets defined in `CMakePresets.json` (with `vcpkg` toolchain configured):

```bash
cd F4SE_plugin
cmake --list-presets
cmake --preset <preset-name>
cmake --build --preset <preset-name>
```

### 2. Procedural Audio Synthesizer (`JUCE_plugin`)

1. Open `JUCE_plugin/ProofOfConcept.jucer` in **Projucer**.
2. Export the project to your IDE (e.g., Visual Studio 2022).
3. Build and run the standalone synthesizer application.
4. Launch the game with the compiled F4SE plugin enabled to begin streaming live OSC telemetry.
