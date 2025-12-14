cmake_minimum_required(VERSION 3.5)
project(ExampleModule LANGUAGES CXX)

# Build a shared library for the example module.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The module sources: collect everything under src/ so adding/removing files
# there doesn't require editing this CMakeLists.
file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

if(NOT MODULE_SOURCES)
    message(FATAL_ERROR "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

add_library(ExampleModule SHARED ${MODULE_SOURCES})

# Make sure the module can find FractalAPI.hpp and other headers. Add the
# repository header folder and src so local includes work.
target_include_directories(ExampleModule PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# By default CMake will produce libExampleModule.so on Unix. Some engines
# expect the module file to be named without the lib prefix (e.g. "ExampleModule.so").
# We remove the prefix on Unix to match that convention. If you want the
# traditional lib prefix instead, remove the PREFIX property below.
if(UNIX)
    set_target_properties(ExampleModule PROPERTIES PREFIX "")
endif()

# Ensure the produced file base name is ExampleModule on all platforms; the
# loader will append .so/.dll as appropriate for the platform.
set_target_properties(ExampleModule PROPERTIES OUTPUT_NAME "ExampleModule")

# Optional: export all symbols on Windows to avoid needing an explicit
# DEF file when building a DLL. Not required on Linux.
if(WIN32)
    set_target_properties(ExampleModule PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Installation (optional): place module into lib or modules folder. Uncomment
# to enable installation.
#install(TARGETS ExampleModule LIBRARY DESTINATION lib)

# Usage:
#   mkdir build && cd build
#   cmake ..
#   cmake --build . --config Release
#
# On Linux this will produce `ExampleModule.so` (no lib prefix). On Windows
# the produced file will be `ExampleModule.dll`.
