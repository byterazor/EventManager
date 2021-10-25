#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
#  Copyright 2021 Dominik Meyer <dmeyer@federationhq.de>
#  This file is part of the EventManager distribution hosted at https://gitea.federationhq.de/byterazor/EventManager.git
#

cmake_minimum_required (VERSION 3.1 FATAL_ERROR)
project (EventManager VERSION 0.0.1 LANGUAGES  CXX)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
include(compdb)

find_package (Threads REQUIRED)

option(EM_TESTS "ENABLE/DISABLE all tests for EventManager" ON)
IF(${EM_TESTS})
  message(STATUS "EventManager tests enabled")
  add_subdirectory(libs/Catch2)
  include(CTest)
  include(libs/Catch2/contrib/Catch.cmake)
ELSE()
  message(STATUS "EventManager tests disabled")
ENDIF()


#
# all source files for the server library
#
SET(EVENTMANAGER_SOURCES
  include/EventManager/Event.hpp
  src/EventManager/Event.cpp
  include/EventManager/Participant.hpp
  src/EventManager/Participant.cpp
  include/EventManager/Manager.hpp
  src/EventManager/Manager.cpp
)

add_library(em-objlib OBJECT ${EVENTMANAGER_SOURCES})
set_property(TARGET em-objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(em-objlib
  PUBLIC
          include
  PRIVATE
          src
)
target_link_libraries(em-objlib PUBLIC Threads::Threads)

add_library(eventmanager SHARED $<TARGET_OBJECTS:em-objlib>)
target_include_directories(eventmanager
  PUBLIC
          include
  PRIVATE
          src
)
target_link_libraries(eventmanager PUBLIC Threads::Threads)

add_library(eventmanager-static STATIC $<TARGET_OBJECTS:em-objlib>)
target_include_directories(eventmanager-static
  PUBLIC
          include
  PRIVATE
          src
)
target_link_libraries(eventmanager-static PUBLIC Threads::Threads)

IF(${EM_TESTS})
#
# add tests as executable
#
add_executable(test_event tests/test_event.cpp)
target_link_libraries(test_event Catch2::Catch2 eventmanager-static)
catch_discover_tests(test_event)

add_executable(test_basic tests/test_basic.cpp)
target_link_libraries(test_basic Catch2::Catch2 eventmanager-static)
catch_discover_tests(test_basic)

ENDIF()
