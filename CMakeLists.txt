# Copyright 2015 The RE2 Authors.  All Rights Reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

# Old enough to support Ubuntu Trusty.
cmake_minimum_required(VERSION 3.2)

######### Hunter VERSION - START

string(COMPARE NOTEQUAL "$ENV{TRAVIS_TAG}" "" travis_deploy)
string(COMPARE EQUAL "$ENV{APPVEYOR_REPO_TAG}" "true" appveyor_deploy)

if(travis_deploy)
  set(version "$ENV{TRAVIS_TAG}")
elseif(appveyor_deploy)
  set(version "$ENV{APPVEYOR_REPO_TAG_NAME}")
else()
  set(version "v2017-04-01")
endif()

string(REGEX REPLACE "^v" "" version "${version}")
string(REGEX REPLACE "-"  "." version "${version}")
string(STRIP ${version} version)                    # https://cmake.org/cmake/help/v3.7/command/string.html

#################################### Hunter VERSION - END

######### Hunter GATE - START

include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.18.18.tar.gz"
  SHA1 "ae1f72b9e8779a58f763179c77f609c0587d26cd"
)

message("\n 
  ------------------------------ PREPARE - HUNTER ----------------------
  VERSION='${version}'
  appveyor_deploy='${appveyor_deploy}'
  travis_deploy='${travis_deploy}'
  -------------------------------------------------------------------\n")

#################################### Hunter GATE - END

project(RE2 VERSION ${version} LANGUAGES CXX) #  LANGUAGES CXX


######### Hunter OPTIONS - START

option(${PROJECT_NAME}_INSTALL  "install package"                   ON)

option(BUILD_STATIC             "build static libraries"            ON)
option(BUILD_SHARED             "build shared libraries"            ON)
set(BUILD_SHARED_LIBS ${BUILD_SHARED}) # option(BUILD_SHARED_LIBS    "build shared libraries" OFF)

option(BUILD_TESTS          "build tests"                           OFF)
option(BUILD_BENCHMARK      "build becnhmarking targets"            OFF)

if(BUILD_TESTS)
  include(CTest)
endif(BUILD_TESTS)

#################################### Hunter OPTIONS - END

option(USEPCRE              "use PCRE in tests and benchmarks"  OFF)

######### Hunter SUMMARY - START

message("\n 
  ------------------------------ ${PROJECT_NAME} v${PROJECT_VERSION} - HUNTER ----------------------
  PROJECT_LANGUAGES='${PROJECT_LANGUAGES}'
  BUILD_STATIC='${BUILD_STATIC}'
  BUILD_SHARED='${BUILD_SHARED}'
  BUILD_SHARED_LIBS='${BUILD_SHARED_LIBS}'
  BUILD_TESTS='${BUILD_TESTS}'
  BUILD_BENCHMARK='${BUILD_BENCHMARK}'
  -------------------------------------------------------------------
\n")

#################################### Hunter SUMMARY - END

set(EXTRA_TARGET_LINK_LIBRARIES)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if(MSVC_VERSION LESS 1800)
    message(FATAL_ERROR "you need Visual Studio 2013 or later")
  endif()
  if(BUILD_SHARED_LIBS)
    # See http://www.kitware.com/blog/home/post/939 for details.
    cmake_minimum_required(VERSION 3.4)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-std=c++11)
endif()

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE -DSTRICT -DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
elseif(UNIX)
  add_compile_options(-pthread)
  list(APPEND EXTRA_TARGET_LINK_LIBRARIES -pthread)
endif()

#if(USEPCRE)
#  add_definitions(-DUSEPCRE)
#  list(APPEND EXTRA_TARGET_LINK_LIBRARIES pcre)
#endif()

######### Hunter PACKAGES - START

if(USEPCRE)
  hunter_add_package(pcre)
  if(HUNTER_ENABLED)
    find_package(pcre CONFIG REQUIRED)
    add_definitions(-DUSEPCRE)
    list(APPEND EXTRA_TARGET_LINK_LIBRARIES pcre::pcre)
  else()
    find_package(pcre REQUIRED)
    add_definitions(-DUSEPCRE)
    list(APPEND EXTRA_TARGET_LINK_LIBRARIES pcre)
  endif(HUNTER_ENABLED)
endif(USEPCRE)

#################################### Hunter PACKAGES - END

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(RE2_SOURCES
    re2/bitstate.cc
    re2/compile.cc
    re2/dfa.cc
    re2/filtered_re2.cc
    re2/mimics_pcre.cc
    re2/nfa.cc
    re2/onepass.cc
    re2/parse.cc
    re2/perl_groups.cc
    re2/prefilter.cc
    re2/prefilter_tree.cc
    re2/prog.cc
    re2/re2.cc
    re2/regexp.cc
    re2/set.cc
    re2/simplify.cc
    re2/stringpiece.cc
    re2/tostring.cc
    re2/unicode_casefold.cc
    re2/unicode_groups.cc
    util/rune.cc
    util/strutil.cc
    )

######### Hunter WRAP - START

set(PROJECT_TARGETS )

if(NOT BUILD_STATIC AND NOT BUILD_SHARED)

  add_library(re2 ${RE2_SOURCES})             # use of BUILD_SHARED_LIBS argument as a fallback
  list(APPEND PROJECT_TARGETS re2)

else()

  # static 
  if(BUILD_STATIC)
    add_library(re2 STATIC ${RE2_SOURCES})
    list(APPEND PROJECT_TARGETS re2)
  endif(BUILD_STATIC)

  # shared
  if(BUILD_SHARED)
    add_library(re2_s SHARED ${RE2_SOURCES})
    set_target_properties(re2_s PROPERTIES OUTPUT_NAME re2)
    list(APPEND PROJECT_TARGETS re2_s)
  endif(BUILD_SHARED)

endif(NOT BUILD_STATIC AND NOT BUILD_SHARED)

# required libraries
# list(APPEND CMAKE_REQUIRED_LIBRARIES rt)

#################################### Hunter WRAP - END 

if(BUILD_TESTS)
    set(TESTING_SOURCES
        re2/testing/backtrack.cc
        re2/testing/dump.cc
        re2/testing/exhaustive_tester.cc
        re2/testing/null_walker.cc
        re2/testing/regexp_generator.cc
        re2/testing/string_generator.cc
        re2/testing/tester.cc
        util/pcre.cc
        )

    add_library(testing STATIC ${TESTING_SOURCES})
    list(APPEND PROJECT_TARGETS testing)

    set(TEST_TARGETS
        charclass_test
        compile_test
        filtered_re2_test
        mimics_pcre_test
        parse_test
        possible_match_test
        re2_test
        re2_arg_test
        regexp_test
        required_prefix_test
        search_test
        set_test
        simplify_test
        string_generator_test

        dfa_test
        exhaustive1_test
        exhaustive2_test
        exhaustive3_test
        exhaustive_test
        random_test
        )
endif(BUILD_TESTS)

if(BUILD_BENCHMARK)

    set(BENCHMARK_TARGETS
        regexp_benchmark
        )

    foreach(target ${TEST_TARGETS})
      add_executable(${target} re2/testing/${target}.cc util/test.cc)
      list(APPEND PROJECT_TARGETS testing)
      target_link_libraries(${target} testing re2 ${EXTRA_TARGET_LINK_LIBRARIES})
      add_test(NAME ${target} COMMAND ${target})
    endforeach(target)

    foreach(target ${BENCHMARK_TARGETS})
      add_executable(${target} re2/testing/${target}.cc util/benchmark.cc)
      target_link_libraries(${target} testing re2 ${EXTRA_TARGET_LINK_LIBRARIES})
    endforeach(target)
endif(BUILD_BENCHMARK)

set(RE2_HEADERS
    re2/filtered_re2.h
    re2/re2.h
    re2/set.h
    re2/stringpiece.h
    )

######### Hunter INSTALL - START

# install(FILES ${RE2_HEADERS} DESTINATION include/re2)
# install(TARGETS re2 ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)

###############
### Install ###
###############

if(NOT ${PROJECT_NAME}_INSTALL)
  return()
endif()

# Namespace RE2::re2

### Install ###
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::") 

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Note: use 'targets_export_name'
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${project_config}"
     INSTALL_DESTINATION "${config_install_dir}"
)

install(
    TARGETS ${PROJECT_TARGETS}
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "${include_install_dir}"
  )

install(
    FILES
    ${RE2_HEADERS}
    DESTINATION "${include_install_dir}/${PROJECT_NAME}"
  )

install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
  )

install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
  )

#################################### Hunter INSTALL - END
