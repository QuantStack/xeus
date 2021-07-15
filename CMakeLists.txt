############################################################################
# Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          #
# Copyright (c) 2016, QuantStack                                           #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.8)
project(xeus)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(XEUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(XEUS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(XEUS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Versionning
# ===========

# Project version
file(STRINGS "${XEUS_INCLUDE_DIR}/xeus/xeus.hpp" xeus_version_defines
     REGEX "#define XEUS_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xeus_version_defines})
    if(ver MATCHES "#define XEUS_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XEUS_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(XEUS_VERSION
    ${XEUS_VERSION_MAJOR}.${XEUS_VERSION_MINOR}.${XEUS_VERSION_PATCH})
message(STATUS "xeus version: v${XEUS_VERSION}")

# Binary version
# See the following URL for explanations about the binary versionning
# https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html#Updating-version-info
file(STRINGS "${XEUS_INCLUDE_DIR}/xeus/xeus.hpp" xeus_version_defines
    REGEX "#define XEUS_BINARY_(CURRENT|REVISION|AGE)")
foreach(ver ${xeus_version_defines})
    if(ver MATCHES "#define XEUS_BINARY_(CURRENT|REVISION|AGE) +([^ ]+)$")
        set(XEUS_BINARY_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(XEUS_BINARY_VERSION
    ${XEUS_BINARY_CURRENT}.${XEUS_BINARY_REVISION}.${XEUS_BINARY_AGE})
message(STATUS "xeus binary version: v${XEUS_BINARY_VERSION}")

# Build options
# =============

# Compilation options
option(XEUS_DISABLE_ARCH_NATIVE "disable -march=native flag" OFF)
option(XEUS_BUILD_SHARED_LIBS "Build xeus shared library." ON)
option(XEUS_BUILD_STATIC_LIBS "Build xeus static library (default if BUILD_SHARED_LIBS is OFF)." ON)
option(XEUS_STATIC_DEPENDENCIES "link statically with xeus dependencies" OFF)

# Test options
option(XEUS_BUILD_TESTS "xeus test suite" OFF)
option(XEUS_DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)

# Static build configuration
# ==========================

if (XEUS_STATIC_DEPENDENCIES)
    set(CPPZMQ_TARGET_NAME cppzmq-static)
    set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "Linking statically with OpenSSL")
else()
    set(CPPZMQ_TARGET_NAME cppzmq)
    set(OPENSSL_USE_STATIC_LIBS OFF CACHE BOOL "Not linking statically with OpenSSL")
endif()

# Dependencies
# ============

set(nlohmann_json_REQUIRED_VERSION 3.2.0)
set(xtl_REQUIRED_VERSION 0.7)
set(cppzmq_REQUIRED_VERSION 4.4.1)
set(zeromq_REQUIRED_VERSION 4.3.2)

if (NOT TARGET nlohmann_json)
    find_package(nlohmann_json ${nlohmann_json_REQUIRED_VERSION} REQUIRED)
endif ()

if (NOT TARGET xtl)
    find_package(xtl ${xtl_REQUIRED_VERSION} REQUIRED)
endif ()

if (NOT TARGET cppzmq AND NOT TARGET cppzmq-static)
    find_package(cppzmq ${cppzmq_REQUIRED_VERSION} REQUIRED)
endif ()

if (NOT TARGET libzmq AND NOT TARGET libzmq-static)
    if (WIN32)
        find_package(zeromq ${zeromq_REQUIRED_VERSION} REQUIRED)
    else ()
        find_package(zeromq ${zeromq_REQUIRED_VERSION} QUIET)

        if (NOT ZeroMQ_FOUND)
            message(STATUS "CMake libzmq package not found, trying again with pkg-config")
            find_package(PkgConfig)
            pkg_check_modules(ZeroMQ libzmq>=${zeromq_REQUIRED_VERSION} REQUIRED)
            set(ZeroMQ_VERSION ${PC_LIBZMQ_VERSION})
            find_library(ZeroMQ_LIBRARY NAMES libzmq.so libzmq.dylib libzmq.dll
                 PATHS ${PC_LIBZMQ_LIBDIR} ${PC_LIBZMQ_LIBRARY_DIRS})
            find_library(ZeroMQ_STATIC_LIBRARY NAMES libzmq-static.a libzmq.a libzmq.dll.a
                 PATHS ${PC_LIBZMQ_LIBDIR} ${PC_LIBZMQ_LIBRARY_DIRS})
            message(STATUS "STATIC_LIBRARY" {ZeroMQ_LIBRARY})
            message(STATUS "STATIC_STATIC_LIBRARY" {ZeroMQ_STATIC_LIBRARY})
         endif ()
    endif ()
endif ()

if (NOT DEFINED OPENSSL_LIBRARY)
    set(OPENSSL_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
    find_package(OpenSSL REQUIRED)
endif ()

# Source files
# ============

set(XEUS_HEADERS
    ${XEUS_INCLUDE_DIR}/xeus/xauthentication.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xcomm.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xcontrol_messenger.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xdap_tcp_client.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xdebugger.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xdebugger_base.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xeus.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xguid.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xhistory_manager.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xinput.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xinterpreter.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xjson.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xkernel.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xkernel_configuration.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xlogger.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xmessage.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xmiddleware.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xserver.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xserver_control_main.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xserver_shell_main.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xserver_zmq.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xserver_zmq_split.hpp
    ${XEUS_INCLUDE_DIR}/xeus/xsystem.hpp
)

set(XEUS_SOURCES
    ${XEUS_SOURCE_DIR}/xauthentication.cpp
    ${XEUS_SOURCE_DIR}/xcomm.cpp
    ${XEUS_SOURCE_DIR}/xcontrol.hpp
    ${XEUS_SOURCE_DIR}/xcontrol.cpp
    ${XEUS_SOURCE_DIR}/xcontrol_messenger.cpp
    ${XEUS_SOURCE_DIR}/xdap_tcp_client.cpp
    ${XEUS_SOURCE_DIR}/xdebugger.cpp
    ${XEUS_SOURCE_DIR}/xdebugger_base.cpp
    ${XEUS_SOURCE_DIR}/xguid.cpp
    ${XEUS_SOURCE_DIR}/xheartbeat.cpp
    ${XEUS_SOURCE_DIR}/xheartbeat.hpp
    ${XEUS_SOURCE_DIR}/xhistory_manager.cpp
    ${XEUS_SOURCE_DIR}/xinput.cpp
    ${XEUS_SOURCE_DIR}/xin_memory_history_manager.hpp
    ${XEUS_SOURCE_DIR}/xin_memory_history_manager.cpp
    ${XEUS_SOURCE_DIR}/xinterpreter.cpp
    ${XEUS_SOURCE_DIR}/xkernel.cpp
    ${XEUS_SOURCE_DIR}/xkernel_configuration.cpp
    ${XEUS_SOURCE_DIR}/xkernel_core.cpp
    ${XEUS_SOURCE_DIR}/xkernel_core.hpp
    ${XEUS_SOURCE_DIR}/xlogger.cpp
    ${XEUS_SOURCE_DIR}/xlogger_impl.hpp
    ${XEUS_SOURCE_DIR}/xlogger_impl.cpp
    ${XEUS_SOURCE_DIR}/xmessage.cpp
    ${XEUS_SOURCE_DIR}/xmock_interpreter.cpp
    ${XEUS_SOURCE_DIR}/xmock_interpreter.hpp
    ${XEUS_SOURCE_DIR}/xmiddleware.cpp
    ${XEUS_SOURCE_DIR}/xpublisher.cpp
    ${XEUS_SOURCE_DIR}/xpublisher.hpp
    ${XEUS_SOURCE_DIR}/xserver.cpp
    ${XEUS_SOURCE_DIR}/xserver_control_main.cpp
    ${XEUS_SOURCE_DIR}/xserver_shell_main.cpp
    ${XEUS_SOURCE_DIR}/xserver_zmq.cpp
    ${XEUS_SOURCE_DIR}/xserver_zmq_split.cpp
    ${XEUS_SOURCE_DIR}/xshell.hpp
    ${XEUS_SOURCE_DIR}/xshell.cpp
    ${XEUS_SOURCE_DIR}/xstring_utils.hpp
    ${XEUS_SOURCE_DIR}/xsystem.cpp
    ${XEUS_SOURCE_DIR}/xtrivial_messenger.hpp
    ${XEUS_SOURCE_DIR}/xtrivial_messenger.cpp
    ${XEUS_SOURCE_DIR}/xzmq_messenger.hpp
    ${XEUS_SOURCE_DIR}/xzmq_messenger.cpp
)

# Targets and link
# ================

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

if (XEUS_STATIC_DEPENDENCIES AND NOT MSVC AND NOT APPLE)
	# Explicitly finds and links with libsodium.a
	# because it is not exported as a dependency by
	# the static build of libzmq. Remove this when
	# it is fixed upstream.
	set(sodium_USE_STATIC_LIBS ON)
	find_package(sodium REQUIRED)
endif ()

macro(xeus_create_target target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    # Output
    # ======

    add_library(${target_name} ${linkage_upper} ${XEUS_SOURCES} ${XEUS_HEADERS})

    target_include_directories(
        ${target_name}
        PUBLIC $<BUILD_INTERFACE:${XEUS_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(
        ${target_name}
        PUBLIC ${CPPZMQ_TARGET_NAME}
        PUBLIC nlohmann_json::nlohmann_json
        PUBLIC xtl
    )

    target_link_libraries(${target_name} PUBLIC OpenSSL::Crypto)

    if (NOT MSVC)
        if (APPLE)
            target_link_libraries(${target_name} PUBLIC "-framework CoreFoundation")
        else ()
            if (XEUS_STATIC_DEPENDENCIES)
                find_path(LIBUUID_INCLUDE_DIR uuid.h PATH_SUFFIXES uuid)
                find_library(LIBUUID_LIBRARY libuuid.a)
                target_include_directories(${target_name} PRIVATE ${LIBUUID_INCLUDE_DIR})
                target_link_libraries(${target_name} PUBLIC ${LIBUUID_LIBRARY}) 
                target_link_libraries(${target_name} PUBLIC ${sodium_LIBRARY_RELEASE})
            else ()
                find_package(LibUUID REQUIRED)
                target_link_libraries(${target_name} PUBLIC LibUUID::LibUUID)
            endif ()
        endif ()
    endif ()

    if (UNIX)
        # CMake does not compute the version number of so files as libtool
        # does on Linux. Strictly speaking, we should exclude FreeBSD and
        # Apple from this, but that would require having different version
        # numbers depending on the platform. We prefer to follow the
        # libtool pattern everywhere.
        math(EXPR XEUS_BINARY_COMPATIBLE "${XEUS_BINARY_CURRENT} - ${XEUS_BINARY_AGE}")
        set_target_properties(
            ${target_name}
            PROPERTIES
            PUBLIC_HEADER "${XEUS_HEADERS}"
            COMPILE_DEFINITIONS "XEUS_EXPORTS"
            PREFIX ""
            VERSION "${XEUS_BINARY_COMPATIBLE}.${XEUS_BINARY_REVISION}.${XEUS_BINARY_AGE}"
            SOVERSION ${XEUS_BINARY_COMPATIBLE}
            OUTPUT_NAME "lib${output_name}"
        )
    else()
        set_target_properties(
            ${target_name}
            PROPERTIES
            PUBLIC_HEADER "${XEUS_HEADERS}"
            COMPILE_DEFINITIONS "XEUS_EXPORTS"
            PREFIX ""
            VERSION ${XEUS_BINARY_VERSION}
            SOVERSION ${XEUS_BINARY_CURRENT}
            OUTPUT_NAME "lib${output_name}"
        )
    endif()

    # Compilation flags
    # =================

    target_compile_features(${target_name} PRIVATE cxx_std_11)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)

        if (NOT XEUS_DISABLE_ARCH_NATIVE)
            target_compile_options(${target_name} PUBLIC -march=native)
        endif ()

        message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
    endif()

    if (${linkage_upper} STREQUAL "STATIC")
        target_compile_definitions(${target_name} PUBLIC XEUS_STATIC_LIB)
    endif ()

    if (MSVC)
        target_compile_definitions(${target_name} PUBLIC -DNOMINMAX)
        target_compile_options(${target_name} PUBLIC /DGUID_WINDOWS /MP /bigobj)
        target_compile_options(${target_name} PUBLIC /wd4251 /wd4996)
    elseif (APPLE)
        target_compile_definitions(${target_name} PUBLIC -DGUID_CFUUID)
    else ()
        target_compile_definitions(${target_name} PUBLIC -DGUID_LIBUUID)
    endif ()

    if (XEUS_STATIC_DEPENDENCIES AND CMAKE_DL_LIBS)
        target_link_libraries(${target_name} PRIVATE ${CMAKE_DL_LIBS} util rt)
    endif ()

endmacro()

set(xeus_targets "")

if (XEUS_BUILD_SHARED_LIBS)
    xeus_create_target(xeus SHARED xeus)
    if(CMAKE_TARGET_SYSTEM MATCHES "Linux" AND OPENSSL_USE_STATIC_LIBS)
        # Do not reexport OpenSSL symbols from xeus, for libraries
        #   Prevents conflicts with other versions of OpenSSL
        #   loaded in the same process namespace, which can cause
        #   crashes if the versions are not compatible.
        set_target_properties(xeus PROPERTIES LINK_FLAGS "-Wl,--exclude-libs,libcrypto.a")
    endif()
    list(APPEND xeus_targets xeus)
endif ()

if (XEUS_BUILD_STATIC_LIBS)
    # On Windows, a static library should use a different output name
    # to avoid the conflict with the import library of a shared one.
    if (CMAKE_HOST_WIN32)
        xeus_create_target(xeus-static STATIC xeus-static)
    else ()
        xeus_create_target(xeus-static STATIC xeus)
    endif ()

    list(APPEND xeus_targets xeus-static)
endif ()

# Tests
# =====

# We need to control from outside whether we enable testing or not. We cannot
# rely on BUILD_TESTING since it doe snot exist until CTest is included.

include(CTest)

if(XEUS_DOWNLOAD_GTEST OR GTEST_SRC_DIR OR XEUS_BUILD_TESTS)
    set(BUILD_TESTING ON)
    message(STATUS "tests enabled")
else ()
    set(BUILD_TESTING OFF)
    message(STATUS "tests disabled")
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(XEUS_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeusConfig.cmake")

install(TARGETS ${xeus_targets}
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus)

# Makes the project importable from the build directory
export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# Configure 'xeusConfig.cmake' for a build tree
set(XEUS_CONFIG_CODE "####### Expanded from \@XEUS_CONFIG_CODE\@ #######\n")
set(XEUS_CONFIG_CODE "${XEUS_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_CONFIG_CODE "${XEUS_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeusConfig.cmake' for an install tree
set(XEUS_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_CMAKECONFIG_INSTALL_DIR})


write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${XEUS_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindLibUUID.cmake
              DESTINATION ${XEUS_CMAKECONFIG_INSTALL_DIR})
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${XEUS_CMAKECONFIG_INSTALL_DIR})
