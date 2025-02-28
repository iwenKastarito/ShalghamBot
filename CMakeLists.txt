# Create a new CMake build system instead of Visual Studio project files
cmake_minimum_required(VERSION 3.10)
project(ChessGame VERSION 1.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect operating system and set SFML path accordingly
if(WIN32)
    # Windows path
    set(SFML_DIR "C:/SFML/lib/cmake/SFML")
else()
    # Linux/Unix path
    # On Arch Linux, SFML is typically installed in the system path
    # No need to set SFML_DIR explicitly
endif()

# Find SFML with updated component names for version 3.0
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# Define project structure
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

# Add include directories
include_directories(
    ${PROJECT_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/ui
    ${PROJECT_SOURCE_DIR}/bot
)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(chess ${SOURCES})

# Link SFML with updated component names
target_link_libraries(chess sfml-graphics sfml-window sfml-system)

# Make sure resources directory exists and is copied
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/resources")
    message(FATAL_ERROR "Resources directory not found. Please create it.")
endif()

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# We'll add tests later once we have a testing framework set up
# For now, comment out the test executable
#enable_testing()
#add_executable(ChessBotTests
#    tests/TestMain.cpp
#    tests/core/ChessGameTests.cpp
#    tests/bot/ChessBotTests.cpp
#) 