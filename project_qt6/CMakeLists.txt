cmake_minimum_required(VERSION 3.16)
project(ItemEditorQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_VERSION_MAJOR 6)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Gui Core REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/include/otb
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tibiadata
    ${CMAKE_CURRENT_SOURCE_DIR}/include/helpers
)

# Core application sources
set(CORE_SOURCES
    src/main.cpp
    src/core/applicationbase.cpp
    src/core/windowbase.cpp
    src/core/utils.cpp
    src/core/resourcemanager.cpp
    src/core/stylesheetmanager.cpp
    src/core/settingsmanager.cpp
    src/core/application.cpp
    src/core/eventmanager.cpp
    src/core/commandmanager.cpp
)

# UI sources
set(UI_SOURCES
    src/ui/mainwindow.cpp
    src/ui/widgets/clientitemview.cpp
    # src/ui/widgets/itempropertyeditor.cpp  # TODO: Fix implementation
    # src/ui/widgets/spritebrowser.cpp       # TODO: Fix implementation
    src/ui/dialogs/aboutdialog.cpp
    src/ui/dialogs/finditemdialog.cpp
    src/ui/dialogs/enhancedfinditems.cpp
    src/ui/dialogs/preferencesdialog.cpp
    src/ui/dialogs/compareotbdialog.cpp
    src/ui/dialogs/updateotbdialog.cpp
    src/ui/dialogs/spritecandidatesdialog.cpp
)

# UI headers with Q_OBJECT for MOC processing
set(UI_HEADERS
    include/ui/mainwindow.h
    include/ui/widgets/clientitemview.h
    # include/ui/widgets/itempropertyeditor.h  # TODO: Fix implementation
    # include/ui/widgets/spritebrowser.h       # TODO: Fix implementation
    include/ui/dialogs/aboutdialog.h
    include/ui/dialogs/finditemdialog.h
    include/ui/dialogs/enhancedfinditems.h
    include/ui/dialogs/preferencesdialog.h
    include/ui/dialogs/compareotbdialog.h
    include/ui/dialogs/updateotbdialog.h
    include/ui/dialogs/spritecandidatesdialog.h
)

# OTB headers with Q_OBJECT for MOC processing
set(OTB_HEADERS
    include/otb/otbbackup.h
    include/otb/otbcache.h
    include/otb/otbperformance.h
)

set(OTB_SOURCES
    src/otb/binarytree.cpp
    src/otb/otbreader.cpp
    src/otb/otbwriter.cpp
    src/otb/otbwriter_helpers.cpp
    src/otb/otbheader.cpp
    src/otb/otbtypes.cpp
    src/otb/otbbackup.cpp
    src/otb/otbcache.cpp
    src/otb/otbperformance.cpp
    src/otb/otberrors.cpp
    src/otb/otbvalidator.cpp
    src/otb/item.cpp
)

set(TIBIADATA_SOURCES
    src/tibiadata/sprparser.cpp
    src/tibiadata/datparser.cpp
    src/tibiadata/imagesimilarity.cpp
)

set(RESOURCE_FILES
    resources/resources.qrc
)

# Create the main executable
add_executable(ItemEditorQt
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${OTB_HEADERS}
    ${OTB_SOURCES}
    ${TIBIADATA_SOURCES}
    ${RESOURCE_FILES}
)

# Link Qt libraries
target_link_libraries(ItemEditorQt PRIVATE
    Qt::Widgets
    Qt::Gui
    Qt::Core
)

# Platform-specific configurations
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# Installation configuration
install(TARGETS ItemEditorQt
    RUNTIME DESTINATION bin
)