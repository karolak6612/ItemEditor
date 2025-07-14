# Main application executable
qt_add_executable(ItemEditor
    main.cpp
    MainForm.cpp
    MainForm.h
    MainForm.ui
)

# Add subdirectories for component libraries (only existing ones)
add_subdirectory(Controls)
add_subdirectory(Dialogs)
add_subdirectory(Helpers)
add_subdirectory(Properties)
add_subdirectory(Host)         # Plugin system core components implemented
add_subdirectory(PluginInterface) # Plugin interface components implemented

# Link component libraries to main executable
target_link_libraries(ItemEditor PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    Qt6::Xml
    ItemEditorControls
    ItemEditorDialogs
    ItemEditorHelpers
    ItemEditorProperties
    ItemEditorHost              # Plugin system core components implemented
    ItemEditorPluginInterface   # Plugin interface components implemented
)

# Include directories
target_include_directories(ItemEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditor PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    OUTPUT_NAME "ItemEditor"
)

# Windows-specific properties
if(WIN32)
    set_target_properties(ItemEditor PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# macOS-specific properties
if(APPLE)
    set_target_properties(ItemEditor PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Item Editor"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_IDENTIFIER "org.ottools.ItemEditor"
    )
endif()

# Installation
install(TARGETS ItemEditor
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install Qt6 dependencies (Windows/Linux)
if(NOT APPLE)
    find_program(QT_DEPLOY_EXECUTABLE NAMES windeployqt macdeployqt linuxdeployqt)
    if(QT_DEPLOY_EXECUTABLE)
        install(CODE "
            execute_process(
                COMMAND ${QT_DEPLOY_EXECUTABLE} \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/ItemEditor
                WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}
            )
        ")
    endif()
endif()