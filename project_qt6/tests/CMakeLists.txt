# Test builds for OTB system
cmake_minimum_required(VERSION 3.16)
project(OTBTests)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Test Gui)

# Include OTB source directory
include_directories(../include/otb)
include_directories(../include)
include_directories(../src)

# Simple build test
add_executable(BuildTest
    build_test.cpp
)

target_link_libraries(BuildTest Qt6::Core)

# Demo backup executable
add_executable(DemoOtbBackup
    demo_backup.cpp
    ../src/otb/otbbackup_standalone.cpp
    ../src/otb/otbheader.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(DemoOtbBackup Qt6::Core)

# Comprehensive OTB test suite
add_executable(OTBComprehensiveTests
    test_otb_comprehensive.cpp
    ../src/otb/binarytree.cpp
    ../src/otb/otbreader.cpp
    ../src/otb/otbwriter.cpp
    ../src/otb/otbheader.cpp
    ../src/otb/item.cpp
    ../src/otb/otbvalidator.cpp
    ../src/otb/otberrors.cpp
    ../src/otb/otbbackup.cpp
    ../src/otb/otbcache.cpp
    ../src/otb/otbperformance.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(OTBComprehensiveTests Qt6::Core Qt6::Test Qt6::Gui)

# Add other individual test executables
add_executable(TestOtbValidator 
    test_otbvalidator.cpp
    ../src/otb/otbvalidator.cpp
    ../src/otb/otberrors.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(TestOtbValidator Qt6::Core Qt6::Gui)

add_executable(TestBinaryTree
    test_binarytree.cpp
    ../src/otb/binarytree.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(TestBinaryTree Qt6::Core Qt6::Gui)

add_executable(TestOtbBackup
    test_otbbackup.cpp
    ../src/otb/otbbackup.cpp
    ../src/otb/otbheader.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(TestOtbBackup Qt6::Core Qt6::Gui)

add_executable(TestOtbPerformance
    test_otbperformance.cpp
    ../src/otb/otbperformance.cpp
    ../src/otb/otbreader.cpp
    ../src/otb/otbwriter.cpp
    ../src/otb/binarytree.cpp
    ../src/otb/otbheader.cpp
    ../src/otb/item.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(TestOtbPerformance Qt6::Core Qt6::Gui)

# Plugin Testing Framework
add_executable(PluginSystemTests
    plugins/test_plugins.cpp
    ../src/plugins/pluginmanager.cpp
    ../src/plugins/pluginloader.cpp
    ../src/plugins/plugindiscovery.cpp
    ../src/plugins/versionmanager.cpp
    ../src/plugins/pluginsecurity.cpp
    ../src/otb/item.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(PluginSystemTests Qt6::Core Qt6::Test Qt6::Gui)

add_executable(PluginLoaderTests
    plugins/test_plugin_loader.cpp
    ../src/plugins/pluginloader.cpp
    ../src/plugins/plugindiscovery.cpp
    ../src/otb/item.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(PluginLoaderTests Qt6::Core Qt6::Test Qt6::Gui)

add_executable(PluginManagerTests
    plugins/test_plugin_manager.cpp
    ../src/plugins/pluginmanager.cpp
    ../src/plugins/pluginloader.cpp
    ../src/plugins/plugindiscovery.cpp
    ../src/plugins/versionmanager.cpp
    ../src/otb/item.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(PluginManagerTests Qt6::Core Qt6::Test Qt6::Gui)

add_executable(PluginPerformanceTests
    plugins/test_plugin_performance.cpp
    ../src/plugins/pluginmanager.cpp
    ../src/plugins/pluginloader.cpp
    ../src/plugins/plugindiscovery.cpp
    ../src/plugins/versionmanager.cpp
    ../src/otb/item.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(PluginPerformanceTests Qt6::Core Qt6::Test Qt6::Gui Qt6::Concurrent)

add_executable(PluginIntegrationTests
    plugins/test_plugin_integration.cpp
    ../src/plugins/pluginmanager.cpp
    ../src/plugins/pluginloader.cpp
    ../src/plugins/plugindiscovery.cpp
    ../src/plugins/versionmanager.cpp
    ../src/plugins/pluginsecurity.cpp
    ../src/otb/item.cpp
    ../src/otb/otbtypes.cpp
)

target_link_libraries(PluginIntegrationTests Qt6::Core Qt6::Test Qt6::Gui)