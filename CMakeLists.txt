# Set CMake minimum version and CMake policy required by UrhoCommon module
cmake_minimum_required (VERSION 3.2.3)
if (COMMAND cmake_policy)
    # Libraries linked via full path no longer produce linker search paths
    cmake_policy (SET CMP0003 NEW)
    # INTERFACE_LINK_LIBRARIES defines the link interface
    cmake_policy (SET CMP0022 NEW)
    # Disallow use of the LOCATION target property - so we set to OLD as we still need it
    cmake_policy (SET CMP0026 OLD)
    # MACOSX_RPATH is enabled by default
    cmake_policy (SET CMP0042 NEW)
    # Honor the visibility properties for SHARED target types only
    cmake_policy (SET CMP0063 OLD)
endif ()

# Set project name
project (minimal-urho3d)

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules)
# Include UrhoCommon.cmake module after setting project name
include (UrhoCommon)

# Define source files
set (TARGET_NAME ${PROJECT_NAME})
set (URHO3D_PACKAGING 1)

file( GLOB_RECURSE SOURCE_FILES FOLLOW_SYMLINKS src/*.cpp src/*.h )

#SET (SOURCE_FILES
#    ${SOURCE_FILES}
#    src/LoaderTools/ComponentExporter.h
#    src/LoaderTools/ComponentExporter.cpp
#    src/LoaderTools/base64.h
#    src/LoaderTools/base64.cpp
#)

# Setup target with resource copying
setup_main_executable ()
