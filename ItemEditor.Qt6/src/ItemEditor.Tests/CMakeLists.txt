# ItemEditor.Tests - Test projects
find_package(Qt6 REQUIRED COMPONENTS Test)

# Core tests - separate executables for each test file
add_executable(ServerItemTests
    ServerItemTests.cpp
)

target_link_libraries(ServerItemTests 
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    ItemEditor.Core
)

add_executable(OtbReaderTests
    OtbReaderTests.cpp
)

target_link_libraries(OtbReaderTests 
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    ItemEditor.Core
)

add_executable(OtbWriterTests
    OtbWriterTests.cpp
)

target_link_libraries(OtbWriterTests 
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    ItemEditor.Core
)

add_executable(ServerItemListTests
    ServerItemListTests.cpp
)

target_link_libraries(ServerItemListTests 
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    ItemEditor.Core
)

# UI tests
add_executable(UITests
    UITests.cpp
    MainWindowTests.cpp
    ServerItemListWidgetTests.cpp
)

target_link_libraries(UITests 
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    ItemEditor.Core
    ItemEditor.Plugins
)

# Plugin tests - separate executables for each test file
add_executable(PluginTests
    PluginTests.cpp
)

target_link_libraries(PluginTests 
    Qt6::Core
    Qt6::Test
    ItemEditor.Core
    ItemEditor.Plugins
)

add_executable(PluginManagerTests
    PluginManagerTests.cpp
)

target_link_libraries(PluginManagerTests 
    Qt6::Core
    Qt6::Test
    ItemEditor.Core
    ItemEditor.Plugins
)

# Platform-specific test configuration
if(UNIX AND NOT APPLE)
    # Set rpath for test executables on Linux
    set_target_properties(ServerItemTests OtbReaderTests OtbWriterTests 
                         ServerItemListTests UITests PluginTests PluginManagerTests
        PROPERTIES
            INSTALL_RPATH_USE_LINK_PATH TRUE
            INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

# Add tests to CTest
add_test(NAME ServerItemTests COMMAND ServerItemTests)
add_test(NAME OtbReaderTests COMMAND OtbReaderTests)
add_test(NAME OtbWriterTests COMMAND OtbWriterTests)
add_test(NAME ServerItemListTests COMMAND ServerItemListTests)
add_test(NAME UITests COMMAND UITests)
add_test(NAME PluginTests COMMAND PluginTests)
add_test(NAME PluginManagerTests COMMAND PluginManagerTests)

# Set test environment for Linux
if(UNIX AND NOT APPLE)
    set_tests_properties(ServerItemTests OtbReaderTests OtbWriterTests 
                         ServerItemListTests UITests PluginTests PluginManagerTests
        PROPERTIES
            ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_BINARY_DIR}/plugins:\$ENV{LD_LIBRARY_PATH}"
    )
endif()