# ItemEditor.UI - Qt6 UI library
add_library(ItemEditor.UI STATIC
    # Custom Widgets
    ServerItemListWidget.cpp
    ServerItemListWidget.h
    ClientItemWidget.cpp
    ClientItemWidget.h
    PropertyEditorWidget.cpp
    PropertyEditorWidget.h
    
    # Dialogs
    FindItemDialog.cpp
    FindItemDialog.h
    FindItemDialog.ui
    ProgressDialog.cpp
    ProgressDialog.h
    ProgressDialogFactory.cpp
    ProgressDialogFactory.h
    SettingsDialog.cpp
    SettingsDialog.h
    PluginConfigDialog.cpp
    PluginConfigDialog.h
    
    # Main Window
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
)

# ItemEditor.UI - Qt6 UI application
add_executable(ItemEditor
    main.cpp
    
    # Resources
    resources.qrc
)

# Link libraries for UI library
target_link_libraries(ItemEditor.UI 
    Qt6::Core 
    Qt6::Widgets
    ItemEditor.Core
)

# Link libraries for main executable
target_link_libraries(ItemEditor 
    Qt6::Core 
    Qt6::Widgets
    ItemEditor.UI
    ItemEditor.Core
    ItemEditor.Plugins
)

# Set include directories for UI library
target_include_directories(ItemEditor.UI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/ItemEditor.Core
    ${CMAKE_SOURCE_DIR}/src/ItemEditor.Plugins
)

# Set include directories for main executable
target_include_directories(ItemEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/ItemEditor.Core
    ${CMAKE_SOURCE_DIR}/src/ItemEditor.Plugins
)

# Configure Qt Designer integration
qt6_add_resources(ItemEditor "resources"
    PREFIX "/"
    FILES
        icons/application.ico
        themes/dark.qss
)

# Add platform-specific compile definitions
if(UNIX AND NOT APPLE)
    target_compile_definitions(ItemEditor PRIVATE
        LINUX_BUILD=1
        QT_NO_DEBUG_OUTPUT  # Disable debug output in release builds
    )
    
    # Add rpath for finding Qt libraries on Linux
    set_target_properties(ItemEditor PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

# Platform-specific configuration
if(WIN32)
    set_target_properties(ItemEditor PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "ItemEditor"
    )
    
    # Add application icon (disabled for now due to compilation issues)
    # target_sources(ItemEditor PRIVATE ItemEditor.rc)
elseif(UNIX AND NOT APPLE)
    # Linux-specific configuration
    set_target_properties(ItemEditor PROPERTIES
        OUTPUT_NAME "itemeditor"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Link additional libraries required on Linux
    target_link_libraries(ItemEditor 
        ${CMAKE_DL_LIBS}  # For dynamic loading
    )
    
    # Install desktop entry and icon on Linux
    install(TARGETS ItemEditor
        RUNTIME DESTINATION bin
    )
    
    # Create desktop entry file
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/itemeditor.desktop.in
        ${CMAKE_CURRENT_BINARY_DIR}/itemeditor.desktop
        @ONLY
    )
    
    # Install desktop entry (if file exists)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/itemeditor.desktop.in)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/itemeditor.desktop
            DESTINATION share/applications
        )
    endif()
endif()