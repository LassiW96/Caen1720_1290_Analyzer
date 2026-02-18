# V1720 ADC Analyzer — EVIO → ROOT Decoder & GUI (for LaTech CAPS)

[![CMake](https://img.shields.io/badge/CMake-3.9%2B-blue.svg)](https://cmake.org/)
[![ROOT](https://img.shields.io/badge/ROOT-6.30%2B-orange.svg)](https://root.cern/)
[![Qt6](https://img.shields.io/badge/Qt6-6.x-41cd52.svg)](https://www.qt.io/)
[![EVIO](https://img.shields.io/badge/EVIO-v5.3-green.svg)](https://github.com/JeffersonLab/evio)

## Overview

This project provides:

1. **A shared library (`libV1720`)** to decode **Caen V1720 CODA/EVIO output** into **ROOT files** with TTree branches for each ADC channel.
2. **A Qt6 GUI application (`V1720GUI`)** with two tabs:
   - **Decode Tab** — select an EVIO file, choose an output directory, and run the decoder with progress and log output.
   - **Analysis & Visualization Tab** — open a decoded ROOT file, browse histograms, write/load ROOT C++ analysis scripts, and run them with the histogram preview rendered directly in the GUI.

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| [CMake](https://cmake.org/) | 3.9+ | Build system |
| [ROOT](https://root.cern/) | 6.30+ | **Do not use the snap package** — build from source or use the CERN apt repo |
| [Qt6](https://www.qt.io/) | 6.x | Auto-installed via `apt` if not found (Ubuntu 20.04+) |
| [EVIO](https://github.com/JeffersonLab/evio) | 5.3 | Auto-fetched and built if not provided |
| C++17 compiler | GCC 7+ / Clang 5+ | Required standard |

### Quick install (Ubuntu 20.04+)

```bash
# Qt6 (if not already installed)
sudo apt-get install -y qt6-base-dev libqt6widgets6 libqt6gui6 libqt6core6 libqt6charts6-dev

# Build tools
sudo apt-get install -y cmake g++ tar
```

> **Note:** If Qt6 is not found during `cmake`, the build will attempt to install it via `apt-get` automatically.

## Building

```bash
git clone https://github.com/LassiW96/V1720ADCAnalyzer.git
cd V1720ADCAnalyzer
mkdir build && cd build

# Basic build (EVIO fetched automatically)
cmake ..
make -j$(nproc)
sudo make install

# Or, with a system EVIO and custom install prefix
cmake .. -DEVIO_ROOT=/path/to/evio -DCMAKE_INSTALL_PREFIX=/your/install/path
make -j$(nproc)
sudo make install
```

### CMake options

| Option | Default | Description |
|---|---|---|
| `-DEVIO_ROOT=<path>` | *(empty)* | Path to an existing EVIO installation. If omitted, EVIO v5.3 is downloaded and built automatically. |
| `-DCMAKE_INSTALL_PREFIX=<path>` | `/usr/local` | Where `make install` places binaries and libraries. |

## Installed files

| Path | Description |
|---|---|
| `<prefix>/bin/V1720GUI` | Qt6 GUI application |
| `<prefix>/lib/libV1720.so` | Shared decoder library |
| `<prefix>/lib/libV1720_rdict.pcm` | ROOT dictionary |
| `<prefix>/lib/libV1720.rootmap` | ROOT auto-load map |

## Running the GUI

```bash
# From the build directory
./V1720GUI

# Or after install
V1720GUI
```

### Workflow

1. **Decode tab**: Browse for an `.evio` file → set output directory → click **Start Decode**. The decoded ROOT file is created with a `t1` TTree.
2. **Analysis tab**: Browse for the decoded `.root` file → histograms in the file are listed → click one to preview, or write/load a script.
3. **Load Example**: Loads the bundled `resources/temp_script.C` into the script editor.
4. **Run Script**: Executes the script and renders the resulting histogram in the preview pane.

## Using the shared library in standalone scripts

```cpp
R__LOAD_LIBRARY(/usr/local/lib/libV1720.so)

void my_analysis() {
    TFile *f = TFile::Open("output.root", "READ");
    TTree *t = (TTree*)f->Get("t1");
    t->Draw("ch0_peak");
}
```

See `testEvio.cpp` and `resources/temp_script.C` for more examples.

## Project Structure

```
V1720ADCAnalyzer/
├── CMakeLists.txt              # Main build configuration
├── cmake/
│   ├── FetchBuildEVIO.cmake    # Auto-download & build EVIO
│   └── FindOrInstallQt6.cmake  # Find or apt-install Qt6
├── src/gui/
│   ├── main.cpp                # Qt application entry point
│   └── MainWindow.cpp/h        # GUI implementation
├── resources/
│   └── temp_script.C           # Example analysis script
├── V1720EvioDecode.cxx/h       # EVIO → ROOT decoder
├── V1720RootFileSetup.cxx/h    # ROOT TTree branch setup
├── V1720_LinkDef.h             # ROOT dictionary linkdef
└── testEvio.cpp                # Standalone usage example
```