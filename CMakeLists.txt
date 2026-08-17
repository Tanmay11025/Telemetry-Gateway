# tells CMake which minimum version is required
cmake_minimum_required(VERSION 3.16)

# names the project
# declares that this is a C++ project
project(message_gateway LANGUAGES CXX)

# collects all C++ files in src into a variable called SOURCES
# simplest way to include all source files for a small project
file(GLOB SOURCES "src/*.cpp")

# creates an executable named message_gateway
# links all collected source files into that binary
add_executable(message_gateway ${SOURCES})
