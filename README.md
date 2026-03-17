# EVIO Decoder & Analysis GUI — V1720 ADC + V1290 TDC (for LaTech CAPS)

[![CMake](https://img.shields.io/badge/CMake-3.9%2B-blue.svg)](https://cmake.org/)
[![ROOT](https://img.shields.io/badge/ROOT-6.30%2B-orange.svg)](https://root.cern/)
[![Qt6](https://img.shields.io/badge/Qt6-6.x-41cd52.svg)](https://www.qt.io/)
[![EVIO](https://img.shields.io/badge/EVIO-v5.3-green.svg)](https://github.com/JeffersonLab/evio)

## Overview

This project provides:

1. **`libV1720`** — shared library to decode **CAEN V1720 ADC** CODA/EVIO data into ROOT files with per-channel TTree branches.
2. **`libV1290`** — shared library to decode **CAEN V1290A TDC** CODA/EVIO data, computing Δt values (ch0−ch1, ch0−ch2) and storing them in a ROOT file.
3. **`DecoderGUI`** — a Qt6 GUI application with two tabs:
   - **Decode Tab** — select an EVIO file, choose an output directory, and decode with either **Decode ADC (V1720)** or **Decode TDC (V1290)** buttons.
   - **Analysis & Visualization Tab** — open a decoded ROOT file, browse histograms, write/load ROOT C++ analysis scripts, and preview results in the GUI.

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| [CMake](https://cmake.org/) | 3.9+ | Build system |
| [ROOT](https://root.cern/) | 6.30+ | **Do not use the snap package** — build from source or use the CERN apt repo |
| [Qt6](https://www.qt.io/) | 6.x | Auto-installed via `apt` if not found (Ubuntu 22.04+) |
| [EVIO](https://github.com/JeffersonLab/evio) | 5.3 | Auto-fetched and built if not provided |
| C++20 compiler | GCC 10+ / Clang 10+ | Required standard |

### Quick install (Ubuntu 22.04+)

```bash
# Qt6 (if not already installed)
sudo apt-get install -y qt6-base-dev libqt6widgets6 libqt6gui6 libqt6core6 libqt6charts6-dev

# Build tools
sudo apt-get install -y cmake g++ tar
```

> **Note:** If Qt6 is not found during `cmake`, the build will attempt to install it via `apt-get` automatically.

## Building

### Quick build (recommended)

```bash
source rebuild_analyzer.sh
```

This cleans the build/install directories, runs cmake + make + install, and sets up `PATH` and `LD_LIBRARY_PATH` so you can run `DecoderGUI` immediately.

### Manual build

```bash
git clone https://github.com/LassiW96/Caen1720_1290_Analyzer.git
cd Caen1720_1290_Analyzer
mkdir build && cd build

# Basic build (EVIO fetched automatically)
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make -j$(nproc)
make install

# Add install/bin to PATH
export PATH=$(pwd)/../install/bin:$PATH
```

### CMake Presets

A `CMakePresets.json` is included for the VS Code CMake Tools extension. Select the **"Default"** preset — it uses `install/` as the prefix and enables `compile_commands.json` for clangd.

### CMake options

| Option | Default | Description |
|---|---|---|
| `-DEVIO_ROOT=<path>` | *(empty)* | Path to an existing EVIO installation. If omitted, EVIO v5.3 is downloaded and built automatically. |
| `-DCMAKE_INSTALL_PREFIX=<path>` | `/usr/local` | Where `make install` places binaries and libraries. |

## Installed files

| Path | Description |
|---|---|
| `<prefix>/bin/DecoderGUI` | Qt6 GUI application |
| `<prefix>/lib/libV1720.so` | V1720 ADC decoder library |
| `<prefix>/lib/libV1290.so` | V1290 TDC decoder library |
| `<prefix>/lib/libV1720_rdict.pcm` | V1720 ROOT dictionary |
| `<prefix>/lib/libV1290_rdict.pcm` | V1290 ROOT dictionary |
| `<prefix>/lib/libV1720.rootmap` | V1720 auto-load map |
| `<prefix>/lib/libV1290.rootmap` | V1290 auto-load map |
| `<prefix>/share/DecoderGUI/resources/` | Analysis script examples |

## Running the GUI

```bash
# After source rebuild_V1720.sh
DecoderGUI

# Or manually
export PATH=<prefix>/bin:$PATH
DecoderGUI
```

> The installed binary uses `$ORIGIN/../lib` RPATH to find its co-installed libraries, so `LD_LIBRARY_PATH` is not required.

### Workflow

1. **Decode tab**:
   - Browse for an `.evio` file → set output directory.
   - Click **Decode ADC (V1720)** for ADC data → creates a ROOT file with a `t1` TTree (per-channel waveform branches).
   - Click **Decode TDC (V1290)** for TDC data → creates a ROOT file with a `tdc` TTree (`delta_t_ch0_ch1`, `delta_t_ch0_ch2` in ns).
2. **Analysis tab**:
   - Browse for the decoded `.root` file → histograms are listed → click to preview.
   - **Load Example** loads `resources/temp_script.C` (ADC example) into the editor.
   - Use **Load Script…** to load `resources/tdc_delta_t.C` for TDC Δt histograms.
   - **Run Script** executes the script and renders the result in the preview pane.

## Using the libraries in standalone scripts

### V1720 ADC

```cpp
gSystem->Load("libV1720");

void my_adc_analysis() {
    V1720EvioDecode dec("input.evio.0", "output_dir");
    dec.decode();

    TFile *f = TFile::Open("output_dir/input_decoded.root", "READ");
    TTree *t = (TTree*)f->Get("t1");
    t->Draw("ch0_peak");
}
```

### V1290 TDC

```cpp
gSystem->Load("libV1290");

void my_tdc_analysis() {
    V1290EvioDecode dec("tdc_input.evio.0", "output_dir");
    dec.decode();

    TFile *f = TFile::Open("output_dir/tdc_input_tdc_decoded.root", "READ");
    TTree *t = (TTree*)f->Get("tdc");
    t->Draw("delta_t_ch0_ch1");
}
```

See `test_v1290.C` and `resources/tdc_delta_t.C` for more examples.

## Project Structure

```
V1720ADCAnalyzer/
├── CMakeLists.txt              # Main build configuration
├── CMakePresets.json            # VS Code CMake Tools preset
├── rebuild_V1720.sh            # One-step build + install script
├── cmake/
│   ├── FetchBuildEVIO.cmake    # Auto-download & build EVIO
│   └── FindOrInstallQt6.cmake  # Find or apt-install Qt6
├── src/gui/
│   ├── main.cpp                # Qt application entry point
│   └── MainWindow.cpp/h        # GUI implementation (ADC + TDC decode)
├── resources/
│   ├── temp_script.C           # Example ADC analysis script
│   └── tdc_delta_t.C           # TDC Δt histogram script
├── V1720EvioDecode.cxx/h       # V1720 ADC EVIO → ROOT decoder
├── V1720RootFileSetup.cxx/h    # V1720 ROOT TTree branch setup
├── V1720_LinkDef.h             # V1720 ROOT dictionary linkdef
├── V1290EvioDecode.cxx/h       # V1290 TDC EVIO → ROOT decoder
├── V1290RootFileSetup.cxx/h    # V1290 ROOT TTree branch setup
├── V1290_LinkDef.h             # V1290 ROOT dictionary linkdef
├── test_v1290.C                # TDC decode test macro
└── testEvio.cpp                # Standalone ADC usage example
```