############################################################################
# Copyright (c) 2016, Sylvain Corlay, Johan Mabille, Martin Renou          #
# Copyright (c) 2016, QuantStack                                           #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

# Unit tests
# ==========

cmake_minimum_required(VERSION 3.1)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(xeus-test)

    enable_testing()

    find_package(xeus REQUIRED CONFIG)
    find_package(nlohmann_json QUIET CONFIG)

    set(XEUS_TEST_DIR ${CMAKE_CURRENT_LIST_DIR})
endif ()

message(STATUS "Forcing tests build type to Release")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

if(nlohmann_json_FOUND)
  add_definitions(-DHAVE_NLOHMANN_JSON)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Intel)
    add_compile_options(-Wunused-parameter -Wextra -Wreorder -Wconversion -Wsign-conversion)

    CHECK_CXX_COMPILER_FLAG(-march=native HAS_MARCH_NATIVE)
    if (HAS_MARCH_NATIVE)
        add_compile_options(-march=native)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    add_compile_options(/EHsc /MP /bigobj)
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()

if(XEUS_DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    if(XEUS_DOWNLOAD_GTEST)
        # Download and unpack googletest at configure time
        configure_file(downloadGTest.cmake.in googletest-download/CMakeLists.txt)
    else()
        # Copy local source of googletest at configure time
        configure_file(copyGTest.cmake.in googletest-download/CMakeLists.txt)
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                     ${CMAKE_CURRENT_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)

    set(GTEST_INCLUDE_DIRS "${gtest_SOURCE_DIR}/include")
    set(GTEST_BOTH_LIBRARIES  gtest_main gtest)
else()
    find_package(GTest REQUIRED)
endif()

find_package(Threads)

find_program(PYTEST
    NAMES pytest-3 pytest py.test-3 py.test)

include_directories(${GTEST_INCLUDE_DIRS})

set(XEUS_TESTS
    test_xin_memory_history_manager.cpp
    test_unit_kernel.cpp
)

if(nlohmann_json_FOUND)
    # Version up to 3.1.2 export the target `nlohmann_json`
    if(TARGET nlohmann_json)
      set(nlohmann_json_TARGET nlohmann_json)
    # Newer versions export the namespaced target `nlohmann_json::nlohmann_json`
    elseif(TARGET nlohmann_json::nlohmann_json)
      set(nlohmann_json_TARGET nlohmann_json::nlohmann_json)
    endif()
endif()

if (TARGET xeus)
    set(xeus_TARGET xeus)
    message("Found xeus shared library.")
elseif (TARGET xeus-static)
    set(xeus_TARGET xeus-static)
    message("Found xeus static library.")
endif ()

foreach(filename IN LISTS XEUS_TESTS)
    get_filename_component(targetname ${filename} NAME_WE)

    add_executable(${targetname} ${filename})
    target_link_libraries(${targetname} PRIVATE ${xeus_TARGET} ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${nlohmann_json_TARGET})
    
    add_test(NAME ${targetname} COMMAND ${targetname})
endforeach()

if(XEUS_DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    # https://github.com/jupyter-xeus/xeus/pull/126
    # This should really just be post gtest and gtest_main targets, but CMake
    # won't allow us to do that. So do it after the last test (which depends on
    # those anyway.)
    if (XEUS_BUILD_SHARED_LIBS)
        add_custom_command(
            TARGET ${targetname} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:gtest>" "${CMAKE_CURRENT_BINARY_DIR}/")

        add_custom_command(
            TARGET ${targetname} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:gtest_main>" "${CMAKE_CURRENT_BINARY_DIR}/")
    endif ()
endif()

# Test_kernel tests
# =================

set(TEST_KERNEL_SOURCES
    test_interpreter.cpp
    test_interpreter.hpp
    main.cpp)

configure_file(
    "${XEUS_TEST_DIR}/test_kernel/kernel.json.in"
    "kernels/test_kernel/kernel.json"
)

add_executable(test_kernel ${TEST_KERNEL_SOURCES})
target_link_libraries(test_kernel PRIVATE ${xeus_TARGET} ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(test_kernel PRIVATE cxx_std_11)

set(CONNECTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/connection.json)

configure_file(
    "${XEUS_TEST_DIR}/test_kernel.py"
    "${CMAKE_CURRENT_BINARY_DIR}/"
    COPYONLY)

add_test(NAME test_kernel
    COMMAND ${PYTEST} test_kernel.py)
set_tests_properties(test_kernel
    PROPERTIES
    ENVIRONMENT "JUPYTER_PATH=${CMAKE_CURRENT_BINARY_DIR}")

# Test_kernel_control tests
# =========================

set(TEST_KERNEL_SPLIT_SOURCES
    test_interpreter.cpp
    test_interpreter.hpp
    main_control.cpp)

configure_file(
    "${XEUS_TEST_DIR}/test_kernel_control/kernel.json.in"
    "kernels/test_kernel_control/kernel.json"
)

add_executable(test_kernel_control ${TEST_KERNEL_SPLIT_SOURCES})
target_link_libraries(test_kernel_control PRIVATE ${xeus_TARGET} ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(test_kernel_control PRIVATE cxx_std_11)

set(CONNECTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/connection.json)

configure_file(
    "${XEUS_TEST_DIR}/test_kernel_control.py"
    "${CMAKE_CURRENT_BINARY_DIR}/"
    COPYONLY)

add_test(NAME test_kernel_control
    COMMAND ${PYTEST} test_kernel_control.py)
set_tests_properties(test_kernel_control
    PROPERTIES
    ENVIRONMENT "JUPYTER_PATH=${CMAKE_CURRENT_BINARY_DIR}")

# Test_kernel_shell tests
# =======================

set(TEST_KERNEL_SPLIT_SOURCES
    test_interpreter.cpp
    test_interpreter.hpp
    main_shell.cpp)

configure_file(
    "${XEUS_TEST_DIR}/test_kernel_shell/kernel.json.in"
    "kernels/test_kernel_shell/kernel.json"
)

add_executable(test_kernel_shell ${TEST_KERNEL_SPLIT_SOURCES})
target_link_libraries(test_kernel_shell PRIVATE ${xeus_TARGET} ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(test_kernel_shell PRIVATE cxx_std_11)

set(CONNECTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/connection.json)

configure_file(
    "${XEUS_TEST_DIR}/test_kernel_shell.py"
    "${CMAKE_CURRENT_BINARY_DIR}/"
    COPYONLY)

add_test(NAME test_kernel_shell
    COMMAND ${PYTEST} test_kernel_shell.py)
set_tests_properties(test_kernel_shell
    PROPERTIES
    ENVIRONMENT "JUPYTER_PATH=${CMAKE_CURRENT_BINARY_DIR}")
