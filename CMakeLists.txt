cmake_minimum_required(VERSION 3.9)

project(V1720 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# ROOT
# -----------------------------
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist Gpad Graf)

# -----------------------------
# EVIO Handling
# -----------------------------
set(EVIO_ROOT "" CACHE PATH "Path to EVIO installation")

if(EVIO_ROOT)

    message(STATUS "Using system EVIO from: ${EVIO_ROOT}")

    add_library(evio SHARED IMPORTED)

    set_target_properties(evio PROPERTIES
        IMPORTED_LOCATION ${EVIO_ROOT}/lib/libevio.so
        INTERFACE_INCLUDE_DIRECTORIES ${EVIO_ROOT}/include
    )

else()

    message(STATUS "Fetching and building EVIO locally")

    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchBuildEVIO.cmake)

endif()

# -----------------------------
# Library Sources
# -----------------------------
set(HEADERS
    V1720EvioDecode.h
    V1720RootFileSetup.h
)

set(SOURCES
    V1720EvioDecode.cxx
    V1720RootFileSetup.cxx
)

add_library(V1720 SHARED ${SOURCES} ${HEADERS})

target_include_directories(V1720 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# -----------------------------
# ROOT Dictionary
# -----------------------------
ROOT_GENERATE_DICTIONARY(
    V1720Dict
    ${HEADERS}
    MODULE V1720
    LINKDEF V1720_LinkDef.h
)

# -----------------------------
# Link Libraries
# -----------------------------
target_link_libraries(V1720 PUBLIC
    ROOT::Core
    ROOT::RIO
    ROOT::Tree
    evio
)

# -----------------------------
# RPATH (Critical Part)
# -----------------------------

# During build → find local EVIO
set_target_properties(V1720 PROPERTIES
    BUILD_RPATH_USE_ORIGIN TRUE
)

# After install → look in same directory as the library
set_target_properties(V1720 PROPERTIES
    INSTALL_RPATH "$ORIGIN"
)

# -----------------------------
# Install
# -----------------------------
install(TARGETS V1720
    LIBRARY DESTINATION lib
)


if(ROOT_VERSION VERSION_GREATER_EQUAL 6.0)
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libV1720_rdict.pcm"
        "${CMAKE_CURRENT_BINARY_DIR}/libV1720.rootmap"
        DESTINATION lib
    )
endif()

# -----------------------------
# GUI Application
# -----------------------------
# dynamic qt fetch
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindOrInstallQt6.cmake)

set(GUI_SOURCES
    src/gui/main.cpp
    src/gui/MainWindow.cpp
)

set(GUI_HEADERS
    src/gui/MainWindow.h
)

add_executable(V1720GUI ${GUI_SOURCES} ${GUI_HEADERS})

target_link_libraries(V1720GUI PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    V1720
    ROOT::Core
    ROOT::RIO
    ROOT::Tree
    ROOT::Hist
    ROOT::Gpad
    ROOT::Graf
)

# Enable MOC processing
set_target_properties(V1720GUI PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Embed the source-tree path so the binary can always locate resources/temp_script.C
target_compile_definitions(V1720GUI PRIVATE
    V1720_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# Preserve runtime library paths so the installed binary can find Qt6/ROOT/EVIO libs
# even when they live in non-standard locations (e.g. /opt/Qt/...)
set_target_properties(V1720GUI PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

install(TARGETS V1720GUI
    RUNTIME DESTINATION bin
)

install(DIRECTORY resources/
    DESTINATION share/V1720/resources
)
