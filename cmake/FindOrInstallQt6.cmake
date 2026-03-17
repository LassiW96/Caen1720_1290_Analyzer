
# Try to find Qt6 normally first
find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)

if(Qt6_FOUND)
    message(STATUS "Found existing Qt6.")
    return()
endif()

message(STATUS "Qt6 not found. Attempting to install via apt (Ubuntu 20.04+)...")

# Check if apt-get is available (Linux/Ubuntu)
find_program(APT_GET_EXECUTABLE apt-get)

if(NOT APT_GET_EXECUTABLE)
    message(FATAL_ERROR
        "apt-get not found. This auto-install only supports Ubuntu 20.04+.\n"
        "Please install Qt6 manually:\n"
        "  sudo apt-get install -y qt6-base-dev libqt6widgets6 libqt6gui6 libqt6core6\n"
        "Then re-run cmake."
    )
endif()

# Install Qt6 development packages via apt
message(STATUS "Running: sudo apt-get install -y qt6-base-dev libqt6charts6-dev")
execute_process(
    COMMAND sudo apt-get install -y
        qt6-base-dev
        libqt6widgets6
        libqt6gui6
        libqt6core6
        libqt6charts6-dev
    RESULT_VARIABLE APT_INSTALL_RES
)

if(NOT APT_INSTALL_RES EQUAL 0)
    message(FATAL_ERROR
        "Failed to install Qt6 via apt-get (exit code: ${APT_INSTALL_RES}).\n"
        "Please install manually:\n"
        "  sudo apt-get install -y qt6-base-dev libqt6charts6-dev\n"
        "Then re-run cmake."
    )
endif()

# Re-attempt to find Qt6 after installation
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
