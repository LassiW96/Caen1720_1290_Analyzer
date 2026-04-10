# EVIO Decoder & Analysis GUI — V1720 ADC + V1290 TDC + TI Combined (for LaTech CAPS)

[![CMake](https://img.shields.io/badge/CMake-3.9%2B-blue.svg)](https://cmake.org/)
[![ROOT](https://img.shields.io/badge/ROOT-6.30%2B-orange.svg)](https://root.cern/)
[![Qt6](https://img.shields.io/badge/Qt6-6.x-41cd52.svg)](https://www.qt.io/)
[![EVIO](https://img.shields.io/badge/EVIO-v5.3-green.svg)](https://github.com/JeffersonLab/evio)

## Overview

This project provides:

1. **`libV1720`** — shared library to decode **CAEN V1720 ADC** CODA/EVIO data into ROOT files with per-channel TTree branches.
2. **`libV1290`** — shared library to decode **CAEN V1290A TDC** CODA/EVIO data, computing Δt values and storing them in a ROOT file.
3. **`libTI`** — shared library to decode **combined TI-triggered data** (V1720 ADC + V1290 TDC in the same EVIO event) into a single ROOT file with all branches in one row per event.
4. **`DecoderGUI`** — a Qt6 GUI application with two tabs:
   - **Decode Tab** — select an EVIO file, choose an output directory, and decode with **Decode ADC (V1720)**, **Decode TDC (V1290)**, or **Decode TI (ADC+TDC)**.
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
| `<prefix>/lib/libTI.so` | TI combined (ADC+TDC) decoder library |
| `<prefix>/lib/libV1720_rdict.pcm` | V1720 ROOT dictionary |
| `<prefix>/lib/libV1290_rdict.pcm` | V1290 ROOT dictionary |
| `<prefix>/lib/libTI_rdict.pcm` | TI ROOT dictionary |
| `<prefix>/share/DecoderGUI/resources/` | Analysis script examples |

## Running the GUI

```bash
# After source rebuild_analyzer.sh
DecoderGUI

# Or manually
export PATH=<prefix>/bin:$PATH
DecoderGUI
```

> The installed binary uses `$ORIGIN/../lib` RPATH to find its co-installed libraries, so `LD_LIBRARY_PATH` is not required.

### Workflow

1. **Decode tab**:
   - Browse for an `.evio` file → set output directory.
   - Click **Decode ADC (V1720)** for standalone ADC data → creates `*_decoded.root` with a `t1` TTree.
   - Click **Decode TDC (V1290)** for standalone TDC data → creates `*_tdc_decoded.root` with a `tdc` TTree.
   - Click **Decode TI (ADC+TDC)** for combined TI-triggered data → creates `*_ti_decoded.root` with a `ti` TTree containing **all** ADC and TDC branches per event.
2. **Analysis tab**:
   - Browse for the decoded `.root` file → histograms listed → click to preview.
   - Use **Load Script…** to load one of the scripts from `resources/`.
   - **Run Script** executes the script and renders the result in the preview pane.

## TI Combined Tree Branches (`ti` tree)

| Branch | Type | Description |
|---|---|---|
| `eventNumber` | `Int_t` | Sequential event counter |
| `triggerTime` | `Float_t` | TI trigger time |
| `ch0_samples` | `vector<Float_t>` | V1720 ch0 waveform (4096 samples) |
| `ch0_integral` | `Float_t` | ch0 waveform integral |
| `ch0_peak` | `Float_t` | ch0 minimum ADC value |
| `ch0_peakIdx` | `Float_t` | Sample index of ch0 minimum |
| `ch1_samples / ch1_*` | — | Same as above for channel 1 |
| `tdc_refChannel` | `Int_t` | Lowest-numbered TDC channel that fired (delta-t reference) |
| `tdc_refRawTime` | `Int_t` | Raw TDC count of the reference channel |
| `tdc_hit_channels` | `vector<Int_t>` | Other TDC channels that fired |
| `tdc_delta_t` | `vector<Float_t>` | Δt in **ns** relative to `tdc_refChannel` |

> **Note:** `tdc_refChannel` is determined per-event as the lowest-numbered channel with a leading-edge hit — it may be ch0, ch1, or any other channel depending on the run configuration.

## Using the libraries in standalone scripts

### V1720 ADC

```cpp
gSystem->Load("libV1720");
V1720EvioDecode dec("input.evio.0", "output_dir");
dec.decode();
TFile *f = TFile::Open("output_dir/input_decoded.root", "READ");
TTree *t = (TTree*)f->Get("t1");
t->Draw("ch0_peak");
```

### V1290 TDC

```cpp
gSystem->Load("libV1290");
V1290EvioDecode dec("tdc_input.evio.0", "output_dir");
dec.decode();
TFile *f = TFile::Open("output_dir/tdc_input_tdc_decoded.root", "READ");
TTree *t = (TTree*)f->Get("tdc");
t->Draw("delta_t");
```

### TI Combined (ADC + TDC)

```cpp
gSystem->Load("libTI");
TIDecode dec("ti_input.evio.0", "output_dir");
dec.decode();
TFile *f = TFile::Open("output_dir/ti_input_ti_decoded.root", "READ");
TTree *t = (TTree*)f->Get("ti");
t->Draw("ch0_peak");      // V1720 ch0 waveform minimum
t->Draw("tdc_delta_t");   // TDC Δt for all hit channels
```

## Analysis Scripts (`resources/`)

| Script | Tree | Description |
|---|---|---|
| `temp_script.C` | `t1` | Example ADC analysis script (loaded by "Load Example" button) |
| `multiple_waveforms.C` | `t1` | Plot 10 raw ADC waveforms |
| `multiple_waveforms_calibrated.C` | `t1` | 10 ADC waveforms with pedestal subtraction & mV calibration |
| `single_waveform.C` | `t1` | Single-event ADC waveform display |
| `tdc_delta_t.C` | `tdc` | Basic TDC Δt histogram |
| `tdc_delta_t_new.C` | `tdc` | TDC Δt multi-channel overlay |
| `tdc_delta_t_with_stats.C` | `tdc` | TDC Δt with statistics printout |
| `ti_multiple_waveforms.C` | `ti` | **TI** — V1720 waveforms, X-axis 0–500 samples, with pedestal & mV calibration |
| `ti_delta_t.C` | `ti` | **TI** — TDC Δt overlay, reference = first hit channel per event |

## Project Structure

```
Caen1720_1290_Analyzer/
├── CMakeLists.txt               # Main build configuration
├── CMakePresets.json            # VS Code CMake Tools preset
├── rebuild_analyzer.sh          # One-step build + install script
├── cmake/
│   ├── FetchBuildEVIO.cmake    # Auto-download & build EVIO
│   └── FindOrInstallQt6.cmake  # Find or apt-install Qt6
├── src/gui/
│   ├── main.cpp                # Qt application entry point
│   └── MainWindow.cpp/h        # GUI (ADC + TDC + TI decode)
├── resources/
│   ├── temp_script.C           # Example ADC analysis script
│   ├── multiple_waveforms_calibrated.C
│   ├── tdc_delta_t_new.C
│   ├── ti_multiple_waveforms.C # TI waveform script (0-500 sample range)
│   └── ti_delta_t.C            # TI TDC Δt script (dynamic ref channel)
├── V1720EvioDecode.cxx/h       # V1720 ADC EVIO → ROOT decoder
├── V1720RootFileSetup.cxx/h    # V1720 ROOT TTree branch setup
├── V1720_LinkDef.h
├── V1290EvioDecode.cxx/h       # V1290 TDC EVIO → ROOT decoder
├── V1290RootFileSetup.cxx/h    # V1290 ROOT TTree branch setup
├── V1290_LinkDef.h
├── TIDecode.cxx/h              # TI combined EVIO → ROOT decoder
├── TIRootFileSetup.cxx/h       # TI combined ROOT TTree branch setup
└── TI_LinkDef.h
```
