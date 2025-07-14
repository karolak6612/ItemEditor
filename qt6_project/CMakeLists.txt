cmake_minimum_required(VERSION 3.16)

project(ItemEditor VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Linux-only build - remove Windows/macOS specific settings
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build performance optimizations
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Qt6 BEFORE calling qt_standard_project_setup()
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Network
    Xml
)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Linux/GCC optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
endif()

# Apply standard Qt project setup AFTER finding Qt6
qt_standard_project_setup()

# Add subdirectories for production build
add_subdirectory(src)
add_subdirectory(plugins)

# Installation configuration
include(GNUInstallDirs)

# Install plugins to correct location
set(INSTALL_PLUGINDIR "${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins")

# Set global properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Linux-specific settings
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Print configuration summary
message(STATUS "ItemEditor Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt6 version: ${Qt6_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Plugin directory: ${INSTALL_PLUGINDIR}")