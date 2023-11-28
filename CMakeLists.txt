cmake_minimum_required(VERSION 3.26)
project(libmodbus_cmake_test)

set(CMAKE_CXX_STANDARD 17)

# ---------------------------------------------------------------------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# Windows DLLs are "runtime" for CMake. Output them to "bin" like the Visual Studio projects do.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Reset output dirs for multi-config builds
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach(OUTPUTCONFIG)


# ---------------------------------------------------------------------------------------


add_subdirectory(3rdparty/libmodbus)


add_executable(libmodbus_cmake_test main.cpp)
target_link_libraries(libmodbus_cmake_test PRIVATE modbus)
target_compile_definitions(libmodbus_cmake_test PRIVATE _CRT_SECURE_NO_WARNINGS)
