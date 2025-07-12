cmake_minimum_required(VERSION 3.16)
project(ItemEditorQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the QT_VERSION_MAJOR variable, used by qt_find_package
set(QT_VERSION_MAJOR 6)

# Find Qt6
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

# Enable CMAKE_AUTOUIC, CMAKE_AUTOMOC, CMAKE_AUTORCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add include directories (Qt modules)
target_include_directories(ItemEditorQt PUBLIC
    $<TARGET_PROPERTY:Qt::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
)

# Add source files, including the resource file
add_executable(ItemEditorQt
    resources.qrc # Added QRC file
    main.cpp
    otb/binarytree.cpp
    otb/otbreader.cpp
    otb/otbwriter.cpp
    otb/otbtypes.cpp
    otb/item.cpp
    mainwindow.cpp
    dialogs/aboutdialog.cpp
    dialogs/spritecandidatesdialog.cpp
    dialogs/finditemdialog.cpp
    dialogs/preferencesdialog.cpp
    dialogs/updateotbdialog.cpp    # Add UpdateOtbDialog
    plugins/iplugin.cpp
    plugins/dummyplugin.cpp
    plugins/realplugin770.cpp
    widgets/clientitemview.cpp
    tibiadata/sprparser.cpp
    tibiadata/datparser.cpp
    tibiadata/imagesimilarity.cpp # Added ImageSimilarity
)

# Add include directories for subfolders
target_include_directories(ItemEditorQt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/otb
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/tibiadata
)


# Link Qt libraries
target_link_libraries(ItemEditorQt PRIVATE
    Qt::Widgets
)

# Specify Windows specific settings
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    # Add other Windows-specific configurations if needed
endif()

install(TARGETS ItemEditorQt
    RUNTIME DESTINATION bin
)
