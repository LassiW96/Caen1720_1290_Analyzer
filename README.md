# Shared Library to Create a ROOT File from CaenV1720 CODA Output

[![CMake](https://img.shields.io/badge/CMake-3.9%2B-blue.svg)](https://cmake.org/)  
[![ROOT](https://img.shields.io/badge/ROOT-6.36%2B-orange.svg)](https://root.cern/)  
[![EVIO](https://img.shields.io/badge/EVIO-v4-green.svg)](https://github.com/codaclub/evio)

## Overview
This project provides a **shared library** to convert **Caen V1720 CODA output** into a **ROOT file**, making it easier to analyze and visualize experimental data.

## Prerequisites
Before building, make sure you have:

- [ROOT](https://root.cern/) version **6.36** or higher  
- [CMake](https://cmake.org/) version **3.9** or higher  
- [EVIO](https://github.com/codaclub/evio) version **4**

## Important Notes
While configuring the project with CMake, you **must** provide:

- **EVIO path:**  
  ```bash
  -DEVIO_ROOT=path/to/evio

- **Install path:**  
  ```bash
  -DCMAKE_INSTALL_PREFIX=path/to/evio

## Building
- Use the following commands to build and install the library:
 ```bash
 mkdir build && cd build
 cmake .. -DEVIO_ROOT=path/to/evio -DCMAKE_INSTALL_PREFIX=install/path
 make -j$(nproc)
 make install

