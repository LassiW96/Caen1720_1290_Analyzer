# Set minimum CMake version [1]
cmake_minimum_required(VERSION 3.9)

# Define the project and package name [1]
set(PACKAGE "V1720")
project(${PACKAGE} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find the ROOT package
find_package(ROOT REQUIRED)

# Set EVIO path
#set(EVIO_ROOT $ENV{HOME}/EVIO/evio/Linux-x86_64)
set(EVIO_ROOT "" CACHE PATH "Path to EVIO installation")

if(NOT EVIO_ROOT)
    message(FATAL_ERROR "EVIO_ROOT not set. Use -DEVIO_ROOT=/path/to/evio")
endif()

message(STATUS "Using EVIO from: ${EVIO_ROOT}")
set(EVIO_INCLUDE_DIR ${EVIO_ROOT}/include)
set(EVIO_LIBRARIES 
            ${EVIO_ROOT}/lib/libevio.so)

# Specify headers and source files
set(HEADERS  
        "V1720EvioDecode.h"
        "V1720RootFileSetup.h")
set(SOURCES 
        "V1720EvioDecode.cxx"
        "V1720RootFileSetup.cxx")

# Create the shared library target [1, 5, 6]
add_library(${PACKAGE} SHARED ${SOURCES} ${HEADERS})

# target_include_directories ensures rootcling finds headers during the build
target_include_directories(${PACKAGE} PUBLIC 
                                ${CMAKE_CURRENT_SOURCE_DIR} 
                                ${EVIO_INCLUDE_DIR})

# Generate the ROOT dictionary
# The MODULE option attaches the dictionary to our existing library target
# The LinkDef file naming follows your requirements
ROOT_GENERATE_DICTIONARY(
    ${PACKAGE}Dict
    ${HEADERS} 
    MODULE ${PACKAGE} 
    LINKDEF ${PACKAGE}_LinkDef.h
)


target_link_libraries(${PACKAGE} PUBLIC 
                                ROOT::Core 
                                ROOT::RIO 
                                ROOT::Tree
                                ${EVIO_LIBRARIES})

# Install artifacts
# You must install the .so, .rootmap, and .pcm files together for ROOT to load them
install(TARGETS ${PACKAGE} DESTINATION lib/)

# For ROOT 6, PCM and rootmap files must stay in the same directory as the library
if (${ROOT_VERSION} VERSION_GREATER_EQUAL 6.0)
    install(FILES 
        "${CMAKE_CURRENT_BINARY_DIR}/lib${PACKAGE}_rdict.pcm"
        "${CMAKE_CURRENT_BINARY_DIR}/lib${PACKAGE}.rootmap"
        DESTINATION lib/
    )
endif()
