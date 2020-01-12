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

set (URHO3D_PACKAGING 1 CACHE BOOL "")
set (URHO3D_DEPLOYMENT_TARGET generic CACHE STRING "") # it is important to set this before running thorugh UrhoCommon

include (UrhoCommon)

# Define source files
set (TARGET_NAME ${PROJECT_NAME})
# options
option(ENABLE_COMPONENT_EXPORTER "Enable Component-Export as JSON" ON)

# get all source files
file( GLOB_RECURSE SOURCE_FILES FOLLOW_SYMLINKS ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp src/*.h )

# create configure file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/project_options.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated/project_options.h @ONLY)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated)

list(APPEND ${SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/project_options.h)

message("created: ${CMAKE_CURRENT_BINARY_DIR}/project_options.h")
message("All Sources: ${SOURCE_FILES}")

# this is due to a problem of qtcreator grouping the files awkwardly since qtcreator 4.11 (nothing mandatory. remove if wanted)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# Setup target with resource copying
setup_main_executable ()
