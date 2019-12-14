# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

execute_process(COMMAND git rev-parse HEAD --sq
    OUTPUT_VARIABLE GIT_SHA1_RAW)
string(STRIP "${GIT_SHA1_RAW}" GIT_SHA1)
add_definitions("-DGIT_SHA1=\"${GIT_SHA1}\"")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hello-world)
