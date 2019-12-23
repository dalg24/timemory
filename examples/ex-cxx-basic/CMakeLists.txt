cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(timemory-CXX-Basic-Example LANGUAGES CXX)

set(timemory_FIND_COMPONENTS_INTERFACE timemory-cxx-basic-example)
set(COMPONENTS cxx compile-options analysis-tools)

option(USE_TAU "Enable TAU support" OFF)
option(USE_PAPI "Enable PAPI support" OFF)
option(USE_CALIPER "Enable Caliper support" OFF)

if(USE_TAU)
    list(APPEND COMPONENTS tau)
endif()
if(USE_PAPI)
    list(APPEND COMPONENTS papi)
endif()
if(USE_CALIPER)
    list(APPEND COMPONENTS caliper)
endif()

find_package(timemory REQUIRED COMPONENTS ${COMPONENTS})
add_executable(ex_cxx_basic ex_cxx_basic.cpp)
target_link_libraries(ex_cxx_basic timemory-cxx-basic-example)
install(TARGETS ex_cxx_basic DESTINATION bin)