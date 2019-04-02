################################################################################
#
#                               Component
#
################################################################################

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME external)


################################################################################
#
#                               MPI
#
################################################################################

if(TIMEMORY_USE_MPI)

    if(WIN32)
        if(EXISTS "C:/Program\ Files\ (x86)/Microsoft\ SDKs/MPI")
            list(APPEND CMAKE_PREFIX_PATH "C:/Program\ Files\ (x86)/Microsoft\ SDKs/MPI")
        endif(EXISTS "C:/Program\ Files\ (x86)/Microsoft\ SDKs/MPI")

        if(EXISTS "C:/Program\ Files/Microsoft\ SDKs/MPI")
            list(APPEND CMAKE_PREFIX_PATH "C:/Program\ Files/Microsoft\ SDKs/MPI")
        endif(EXISTS "C:/Program\ Files/Microsoft\ SDKs/MPI")
    endif()

    # MPI C compiler from environment
    set(_ENV MPICC)
    if(NOT DEFINED MPI_C_COMPILER AND NOT "$ENV{${_ENV}}" STREQUAL "")
        message(STATUS "Setting MPI C compiler to: $ENV{${_ENV}}")
        set(MPI_C_COMPILER $ENV{${_ENV}} CACHE FILEPATH "MPI C compiler")
    endif()

    # MPI C++ compiler from environment
    set(_ENV MPICC)
    if(NOT DEFINED MPI_CXX_COMPILER AND NOT "$ENV{${_ENV}}" STREQUAL "")
        message(STATUS "Setting MPI C++ compiler to: $ENV{${_ENV}}")
        set(MPI_CXX_COMPILER $ENV{${_ENV}} CACHE FILEPATH "MPI C++ compiler")
    endif()

    unset(_ENV)

    find_package(MPI)

    set(MPI_LIBRARIES )
    if(MPI_FOUND)

        # Add the MPI-specific compiler and linker flags
        to_list(_FLAGS "${MPI_C_COMPILE_FLAGS}")
        foreach(_FLAG ${_FLAGS})
            add_c_flag_if_avail("${_FLAG}")
        endforeach()
        unset(_FLAGS)
        to_list(_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
        foreach(_FLAG ${_FLAGS})
            add_cxx_flag_if_avail("${_FLAG}")
            message(STATUS "checking ${_FLAG}")
        endforeach()
        unset(_FLAGS)
        add(CMAKE_EXE_LINKER_FLAGS "${MPI_CXX_LINK_FLAGS}")
        list(APPEND EXTERNAL_INCLUDE_DIRS
            ${MPI_INCLUDE_PATH} ${MPI_C_INCLUDE_PATH} ${MPI_CXX_INCLUDE_PATH})

        foreach(_TYPE C_LIBRARIES CXX_LIBRARIES EXTRA_LIBRARY)
            set(_TYPE MPI_${_TYPE})
            if(${_TYPE})
                list(APPEND EXTERNAL_LIBRARIES ${${_TYPE}})
            endif()
        endforeach()

        list(APPEND ${PROJECT_NAME}_DEFINITIONS TIMEMORY_USE_MPI)

        if(NOT MPIEXEC_EXECUTABLE AND MPIEXEC)
          set(MPIEXEC_EXECUTABLE ${MPIEXEC} CACHE FILEPATH "MPI executable")
        endif()

        if(NOT MPIEXEC_EXECUTABLE AND MPI_EXECUTABLE)
          set(MPIEXEC_EXECUTABLE ${MPI_EXECUTABLE} CACHE FILEPATH "MPI executable")
        endif()

    else()

        set(TIMEMORY_USE_MPI OFF)
        set(TIMEMORY_TEST_MPI OFF)
        message(WARNING "MPI not found. Proceeding without MPI")

    endif()

endif()


################################################################################
#
#                               Threading
#
################################################################################

if(NOT WIN32)
    set(CMAKE_THREAD_PREFER_PTHREAD ON)
endif()

find_package(Threads)

if(THREADS_FOUND AND (WIN32 OR CMAKE_CXX_COMPILER_IS_INTEL))
    list(APPEND EXTERNAL_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()


################################################################################
#
#                               PyBind11
#
################################################################################

if(TIMEMORY_USE_PYTHON_BINDING)

    # checkout PyBind11 if not checked out
    checkout_git_submodule(RECURSIVE
        RELATIVE_PATH source/python/pybind11
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

    # make sure pybind11 gets installed in same place as TiMemory
    if(PYBIND11_INSTALL)
        set(PYBIND11_CMAKECONFIG_INSTALL_DIR
            "${TIMEMORY_INSTALL_DATAROOTDIR}/cmake/pybind11"
            CACHE STRING "install path for pybind11Config.cmake" FORCE)
        set(CMAKE_INSTALL_INCLUDEDIR ${TIMEMORY_INSTALL_INCLUDEDIR}
            CACHE PATH "Include file installation path" FORCE)
    endif()

    # C++ standard
    set(PYBIND11_CPP_STANDARD -std=c++${CMAKE_CXX_STANDARD}
        CACHE STRING "PyBind11 CXX standard" FORCE)

    # add PyBind11 to project
    add_subdirectory(${PROJECT_SOURCE_DIR}/source/python/pybind11)

    if(NOT PYBIND11_PYTHON_VERSION)
        execute_process(COMMAND ${PYTHON_EXECUTABLE}
            -c "import sys; print('{}.{}'.format(sys.version_info[0], sys.version_info[1]))"
            OUTPUT_VARIABLE PYTHON_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        message(STATUS "Python version: ${PYTHON_VERSION}")
        set(PYBIND11_PYTHON_VERSION "${PYTHON_VERSION}"
            CACHE STRING "Python version" FORCE)
    endif(NOT PYBIND11_PYTHON_VERSION)

    add_feature(PYBIND11_PYTHON_VERSION "PyBind11 Python version")

    execute_process(COMMAND ${PYTHON_EXECUTABLE}
        -c "import time ; print('{} {}'.format(time.ctime(), time.tzname[0]))"
        OUTPUT_VARIABLE TIMEMORY_INSTALL_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)

    string(REPLACE "  " " " TIMEMORY_INSTALL_DATE "${TIMEMORY_INSTALL_DATE}")

    ########################################
    #   Python installation directories
    ########################################
    set(TIMEMORY_STAGING_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH
        "Installation prefix (relevant in pip staged builds)")

    if(TIMEMORY_SETUP_PY)

        set(TIMEMORY_INSTALL_PYTHONDIR ${TIMEMORY_STAGING_PREFIX}/timemory CACHE PATH
            "Installation prefix of python" FORCE)

        set(TIMEMORY_INSTALL_FULL_PYTHONDIR
            ${CMAKE_INSTALL_PREFIX}/lib/python${PYBIND11_PYTHON_VERSION}/site-packages/timemory)

        add_feature(TIMEMORY_INSTALL_PYTHONDIR "TiMemory Python installation directory")
        add_feature(TIMEMORY_STAGING_PREFIX "Installation prefix (relevant in pip staged builds)")

    else(TIMEMORY_SETUP_PY)

        set(TIMEMORY_INSTALL_PYTHONDIR
            ${CMAKE_INSTALL_LIBDIR}/python${PYBIND11_PYTHON_VERSION}/site-packages/timemory
            CACHE PATH "Installation directory for python")

        set(TIMEMORY_INSTALL_FULL_PYTHONDIR
            ${CMAKE_INSTALL_PREFIX}/${TIMEMORY_INSTALL_PYTHONDIR})

    endif(TIMEMORY_SETUP_PY)

    set(TIMEMORY_CONFIG_PYTHONDIR
        ${CMAKE_INSTALL_LIBDIR}/python${PYBIND11_PYTHON_VERSION}/site-packages/timemory)

else(TIMEMORY_USE_PYTHON_BINDING)

    set(TIMEMORY_CONFIG_PYTHONDIR ${CMAKE_INSTALL_PREFIX})

endif(TIMEMORY_USE_PYTHON_BINDING)


################################################################################
#
#        Checkout Cereal if not checked out
#
################################################################################

checkout_git_submodule(RECURSIVE
    RELATIVE_PATH source/cereal
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})


################################################################################
#
#        External variables
#
################################################################################

# including the directories
safe_remove_duplicates(EXTERNAL_INCLUDE_DIRS ${EXTERNAL_INCLUDE_DIRS})
safe_remove_duplicates(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES})
list(APPEND ${PROJECT_NAME}_TARGET_INCLUDE_DIRS ${EXTERNAL_INCLUDE_DIRS})


################################################################################
#
#                               Component
#
################################################################################

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME development)
