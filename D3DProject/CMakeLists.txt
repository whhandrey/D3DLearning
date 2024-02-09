cmake_minimum_required (VERSION 3.8)

project ("D3DProject")

set(CMAKE_CXX_STANDARD 20)

# compiler version check
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Using MSVC version, ${MSVC_VERSION}")
else()
    message(FATAL_ERROR "Compiler not supported")
endif()

# Include sub-projects.
add_subdirectory ("source")
