cmake_minimum_required(VERSION 3.16)
project(TestOtbErrors LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_VERSION_MAJOR 6)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui REQUIRED)

set(CMAKE_AUTOMOC ON)

# Set include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otb
)

# Error handling test executable
add_executable(TestOtbErrors
    test_otberrors.cpp
    binarytree.cpp
    otberrors.cpp
    otbvalidator.cpp
    otbheader.cpp
    otbreader.cpp
    otbtypes.cpp
    item.cpp
    otbcache.cpp
    otbperformance.cpp
)

target_link_libraries(TestOtbErrors PRIVATE
    Qt::Core
    Qt::Gui
)

# Error handling integration test executable
add_executable(TestOtbErrorsIntegration
    test_otberrors_integration.cpp
    binarytree.cpp
    otberrors.cpp
    otbvalidator.cpp
    otbheader.cpp
    otbreader.cpp
    otbtypes.cpp
    item.cpp
    otbcache.cpp
    otbperformance.cpp
)

target_link_libraries(TestOtbErrorsIntegration PRIVATE
    Qt::Core
    Qt::Gui
)

# Validation test executable (existing)
add_executable(TestOtbValidator
    test_otbvalidator.cpp
    binarytree.cpp
    otbvalidator.cpp
    otberrors.cpp
    otbheader.cpp
    otbreader.cpp
    otbtypes.cpp
    item.cpp
    otbcache.cpp
    otbperformance.cpp
)

target_link_libraries(TestOtbValidator PRIVATE
    Qt::Core
    Qt::Gui
)

# Performance optimization test executable
add_executable(TestOtbPerformance
    test_otbperformance.cpp
    binarytree.cpp
    otberrors.cpp
    otbvalidator.cpp
    otbheader.cpp
    otbreader.cpp
    otbwriter.cpp
    otbwriter_helpers.cpp
    otbtypes.cpp
    item.cpp
    otbcache.cpp
    otbperformance.cpp
)

target_link_libraries(TestOtbPerformance PRIVATE
    Qt::Core
    Qt::Gui
)

# Simple performance test executable (without Qt signals/slots)
add_executable(TestOtbPerformanceSimple
    test_otbperformance_simple.cpp
    binarytree.cpp
    otberrors.cpp
    otbvalidator.cpp
    otbheader.cpp
    otbreader.cpp
    otbwriter.cpp
    otbwriter_helpers.cpp
    otbtypes.cpp
    item.cpp
    otbcache.cpp
    otbperformance.cpp
)

target_link_libraries(TestOtbPerformanceSimple PRIVATE
    Qt::Core
    Qt::Gui
)

# Minimal performance test executable (only basic functionality)
add_executable(TestPerformanceMinimal
    test_performance_minimal.cpp
    otbperformance.cpp
)

target_link_libraries(TestPerformanceMinimal PRIVATE
    Qt::Core
    Qt::Gui
)

# Basic performance test executable (standalone, no external dependencies)
add_executable(TestPerformanceBasic
    test_performance_basic.cpp
)

target_link_libraries(TestPerformanceBasic PRIVATE
    Qt::Core
    Qt::Gui
)