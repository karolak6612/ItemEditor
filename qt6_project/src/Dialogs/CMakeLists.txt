# ItemEditor Dialogs Library
# Exact mirror of Legacy_App/csharp/Source/Dialogs/

qt_add_library(ItemEditorDialogs STATIC
    AboutForm.cpp
    AboutForm.h
    AboutForm.ui
    CompareOtbForm.cpp
    CompareOtbForm.h
    CompareOtbForm.ui
    FindItemForm.cpp
    FindItemForm.h
    FindItemForm.ui
    NewOtbFileForm.cpp
    NewOtbFileForm.h
    NewOtbFileForm.ui
    PreferencesForm.cpp
    PreferencesForm.h
    PreferencesForm.ui
    ProgressForm.cpp
    ProgressForm.h
    ProgressForm.ui
    UpdateForm.cpp
    UpdateForm.h
    UpdateForm.ui
    UpdateSettingsForm.cpp
    UpdateSettingsForm.h
    UpdateSettingsForm.ui
)

# Link required Qt6 modules
target_link_libraries(ItemEditorDialogs PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
)

# Link to other ItemEditor components
target_link_libraries(ItemEditorDialogs PUBLIC
    ItemEditorOTLibOTB
    ItemEditorHelpers
    ItemEditorPluginInterface
)

# Include directories
target_include_directories(ItemEditorDialogs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorDialogs PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC, UIC, RCC for this library
set_target_properties(ItemEditorDialogs PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)