cmake_minimum_required (VERSION 3.8)

# include 3rd party
include("${CMAKE_SOURCE_DIR}/scripts/fetch_libraries.cmake")

# download 3rd party
FetchContent_MakeAvailable(glfw)

# create executable
file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/source/*.hpp" "${CMAKE_SOURCE_DIR}/source/*.cpp")
source_group("source" "${CMAKE_SOURCE_DIR}/source" ${SUBDIRECTORIES})
# Add source to this project's executable.
add_executable ("source" "main.cpp" ${SUBDIRECTORIES})

# include dirs and link libs
target_include_directories("source" PRIVATE "${CMAKE_SOURCE_DIR}/source")

target_link_libraries("source" PRIVATE glfw)
target_link_libraries("source" PRIVATE d3d11.lib)
target_link_libraries("source" PRIVATE d3dcompiler.lib)
target_link_libraries("source" PRIVATE dxguid.lib)

# debug and release flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions("source" PUBLIC PROJ_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions("source" PUBLIC PROJ_RELEASE)
endif()

# macro definitions
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions("source" PUBLIC APP_API=__declspec\(dllexport\))
    target_compile_definitions("source" PUBLIC FORCEINLINE=__forceinline)
endif()
