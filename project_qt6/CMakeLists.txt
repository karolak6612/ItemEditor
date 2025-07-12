cmake_minimum_required(VERSION 3.16)
project(ItemEditorQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_VERSION_MAJOR 6)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

target_include_directories(ItemEditorQt PUBLIC
    $<TARGET_PROPERTY:Qt::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
)

add_executable(ItemEditorQt
    resources.qrc
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
    dialogs/compareotbdialog.cpp
    dialogs/updateotbdialog.cpp
    plugins/iplugin.cpp
    plugins/dummyplugin.cpp
    plugins/realplugin770.cpp
    widgets/clientitemview.cpp
    tibiadata/sprparser.cpp
    tibiadata/datparser.cpp
    tibiadata/imagesimilarity.cpp
)

target_include_directories(ItemEditorQt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/otb
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/tibiadata
)

target_link_libraries(ItemEditorQt PRIVATE
    Qt::Widgets
)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

install(TARGETS ItemEditorQt
    RUNTIME DESTINATION bin
)
