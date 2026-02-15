# Shared Library to Create a ROOT File from CaenV1720 CODA Output (for LaTech CAPS)

[![CMake](https://img.shields.io/badge/CMake-3.11%2B-blue.svg)](https://cmake.org/)  
[![ROOT](https://img.shields.io/badge/ROOT-6.30%2B-orange.svg)](https://root.cern/)  
[![EVIO](https://img.shields.io/badge/EVIO-v4-green.svg)](https://github.com/codaclub/evio)

## Overview
This project provides a **shared library** to convert **Caen V1720 CODA output** into a **ROOT file**, making it easier to analyze and visualize experimental data.

## Prerequisites
Before building, make sure you have:

- [ROOT](https://root.cern/) version **6.30** or higher (may not work if root was installed via snap) 
- [CMake](https://cmake.org/) version **3.11** or higher  
- [EVIO](https://coda.jlab.org/drupal/content/event-io-evio) version **4** (optional - if not, evio will be downloaded and built)

## Important Notes
While configuring the project with CMake, provide:

- **If EVIO is already installed path (path to your evio headers and libraries):**  
  ```bash
  -DEVIO_ROOT=path/to/evio

  If This variable is not provided, evio will be downloaded and built automatically within the project.

- **Install path (shared library will be here):**  
  ```bash
  -DCMAKE_INSTALL_PREFIX=install/path

## Building
- Clone the repo in a fresh directory:
    ```bash
    git clone https://github.com/LassiW96/V1720ADCAnalyzer.git

- Use the following commands to build and install the library:
    ```bash
    mkdir build && cd build
    cmake .. -DEVIO_ROOT=path/to/evio(optional) -DCMAKE_INSTALL_PREFIX=install/path
    make -j$(nproc)
    make install

## Using the shared library:
- Once the library is generated, it can be used in a root script by simply loading it in the very top of the script as follows:
    ```bash
    R__LOAD_LIBRARY({CMAKE_INSTALL_PREFIX}/lib/libV1720.so)

- There is a sample root script in the repo (testEvio.cpp) take that as a guide.
