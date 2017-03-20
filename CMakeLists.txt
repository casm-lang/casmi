#
#   Copyright (c) 2014-2017 CASM Organization
#   All rights reserved.
#
#   Developed by: Philipp Paulweber
#                 Emmanuel Pescosta
#                 Florian Hahn
#                 https://github.com/casm-lang/casmi
#
#   This file is part of casmi.
#
#   casmi is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   casmi is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with casmi. If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required( VERSION 3.1 )

if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../gtest" )
  set( LIBS .. )
else()
  set( LIBS lib )
endif()

foreach( DEPENDENCY
    stdhl
    pass
    casm-ir
    casm-fe
    casm-tc
    )
  if( NOT TARGET ${DEPENDENCY} )
    add_subdirectory( ${LIBS}/${DEPENDENCY} lib/${DEPENDENCY} )
  endif()  
endforeach()


file( GLOB_RECURSE CXX_SRC_FILES "src/*.cpp" )
file( GLOB_RECURSE CXX_UTS_FILES "uts/*.cpp" )


set( TARGET casmi )
project( ${TARGET} )


include_directories(
  .
  src
  obj
  ${LIBS}/gtest/googletest
  ${LIBS}/gtest/googletest/include
  ${LIBS}/stdhl
  ${LIBS}/pass
  ${LIBS}/casm-ir
  ${LIBS}/casm-fe
  ${LIBS}/casm-tc
)


set( CMAKE_CXX_FLAGS          "${CMAKE_CXX_FLAGS}          -std=c++11 -Wall" )

set( CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS_DEBUG}    -O0" )
set( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE}  -O3" )
set( CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -O1 -Wextra -g -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=undefined -fsanitize=address" )


set( VERSION ${CMAKE_CURRENT_SOURCE_DIR}/obj/version.h )
add_custom_command(
  OUTPUT  ${VERSION}
  COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR} version
  COMMENT ""
)

set( LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/obj/license.h )
add_custom_command(
  OUTPUT  ${LICENSE}
  COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR} license
  DEPENDS ${VERSION} ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt
  COMMENT ""
)


add_executable( ${TARGET}
  ${VERSION}
  ${LICENSE}
  ${CXX_SRC_FILES}
)

target_link_libraries( ${TARGET}
  casm-fe
  casm-ir
  pass
  stdhl
)


add_library( ${TARGET}-test OBJECT
  ${CXX_UTS_FILES}
)

add_executable( ${TARGET}-check
  ${LIBS}/gtest/googletest/src/gtest_main.cc
  ${LIBS}/gtest/googletest/src/gtest-all.cc  
  $<TARGET_OBJECTS:${TARGET}-test>
  $<TARGET_OBJECTS:pass-test>
  $<TARGET_OBJECTS:stdhl-test>
  $<TARGET_OBJECTS:casm-ir-test>
  $<TARGET_OBJECTS:casm-fe-test>
  $<TARGET_OBJECTS:casm-tc-test>
)

target_link_libraries( ${TARGET}-check
  casm-ir
  casm-fe
  pass
  stdhl
  pthread
)


add_library( ${TARGET}-benchmark OBJECT
  ${LIBS}/hayai/src/hayai_posix_main.cpp
)
  
add_executable( ${TARGET}-run
  $<TARGET_OBJECTS:${TARGET}-benchmark>
#  $<TARGET_OBJECTS:pass-benchmark>
#  $<TARGET_OBJECTS:stdhl-benchmark>
#  $<TARGET_OBJECTS:casm-ir-benchmark>
#  $<TARGET_OBJECTS:casm-fe-benchmark>
  $<TARGET_OBJECTS:casm-tc-benchmark>
)

target_link_libraries( ${TARGET}-run
  pthread
)



if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug
    CACHE STRING "Choose the type of build : debug sanitize release."
    FORCE
  )
endif( NOT CMAKE_BUILD_TYPE )


message( "* ${CMAKE_BUILD_TYPE} : ${TARGET} : ${LIBS}" )
