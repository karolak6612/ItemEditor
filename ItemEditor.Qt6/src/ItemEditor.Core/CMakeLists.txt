# ItemEditor.Core - Business logic and data models
add_library(ItemEditor.Core STATIC
    # Data Models
    ServerItem.cpp
    ServerItem.h
    ClientItem.cpp
    ClientItem.h
    ServerItemList.cpp
    ServerItemList.h
    
    # File I/O
    OtbReader.cpp
    OtbReader.h
    OtbWriter.cpp
    OtbWriter.h
    OtbFileManager.cpp
    OtbFileManager.h
    OtbFileValidator.cpp
    OtbFileValidator.h
    
    # Backup and Recovery
    BackupManager.cpp
    BackupManager.h
    
    # Error Handling
    ErrorHandler.cpp
    ErrorHandler.h
    
    # Core Types
    ItemTypes.h
    ItemEnums.h
    
    # Validation
    ItemValidator.cpp
    ItemValidator.h
    
    # Undo/Redo System
    UndoCommands.cpp
    UndoCommands.h
    
    # Item Editing Management
    ItemEditingManager.cpp
    ItemEditingManager.h
    
    # Item Comparison
    ItemComparator.cpp
    ItemComparator.h
)

target_link_libraries(ItemEditor.Core Qt6::Core Qt6::Gui Qt6::Widgets)

# Platform-specific linking
if(UNIX AND NOT APPLE)
    # Link additional libraries required on Linux
    target_link_libraries(ItemEditor.Core 
        ${CMAKE_DL_LIBS}  # For dynamic loading
        m                 # Math library
    )
    
    # Set position independent code for shared libraries
    set_target_properties(ItemEditor.Core PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Set include directories
target_include_directories(ItemEditor.Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)