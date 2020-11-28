
list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src_addon)

#include("CMake/Editor.cmake")
add_subdirectory(src_addon/Navigation)
link_libraries(Navigation)


add_subdirectory(src_addon/Editor)
link_libraries(Editor)

#include("CMake/Navigation.cmake")

message("_________________ ADDONS CMAKE ${INCLUDE_DIRS}")

