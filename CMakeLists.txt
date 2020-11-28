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

include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.214.tar.gz"
    SHA1 "e14bc153a7f16d6a5eeec845fb0283c8fad8c358"
)

# Set project name
project (urho3d-blender-runtime)


# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
# Include UrhoCommon.cmake module after setting project name

set (URHO3D_PACKAGING 0 CACHE BOOL "")
set (URHO3D_DEPLOYMENT_TARGET generic CACHE STRING "") # it is important to set this before running thorugh UrhoCommon

include (UrhoCommon)

# include zeromq via hunter-package-manager
hunter_add_package(ZeroMQ)
find_package(ZeroMQ CONFIG REQUIRED)
# ----

# Define source files
set (TARGET_NAME ${PROJECT_NAME})
# options
option(GAME_ENABLE_DEBUG_TOOLS "Enable Debug-Tools(Editor,Console,Profiler)" ON)
option(GAME_ENABLE_COMPONENT_EXPORTER "Enable Component-Export as JSON" ON)
option(GAME_RETRIEVE_GIT_HASH "Retrieve current-githash" ON)
option(GAME_DEBUGGING "Active GAME_DEBUGGING-Define" OFF)

set(GIT_HASH "unknown")

if (GAME_RETRIEVE_GIT_HASH)
    ## see: https://coderefinery.github.io/cmake/07-exercise/

    # find Git and if available set GIT_HASH variable
    find_package(Git QUIET)
    if(GIT_FOUND)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} --no-pager show -s --pretty=format:%h -n 1
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY
            ${PROJECT_SOURCE_DIR}
        ERROR_QUIET
        )
    endif()
endif()

source_group("Source" ${PROJECT_SOURCE_DIR})

# get all source files
file( GLOB_RECURSE SOURCE_FILES FOLLOW_SYMLINKS ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp src/*.h src/*.hpp)

# create configure file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/project_options.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated/project_options.h @ONLY)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd/cppzmq )

list(APPEND ${SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/project_options.h)

message("created: ${CMAKE_CURRENT_BINARY_DIR}/project_options.h")


# Setup target with resource copying
setup_main_executable ()
target_link_libraries(${TARGET_NAME} ZeroMQ::libzmq-static)
