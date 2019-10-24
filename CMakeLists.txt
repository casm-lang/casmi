#
#   Copyright (C) 2014-2019 CASM Organization <https://casm-lang.org>
#   All rights reserved.
#
#   Developed by: Philipp Paulweber
#                 Emmanuel Pescosta
#                 Florian Hahn
#                 <https://github.com/casm-lang/casmi>
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

set( PROJECT casmi )

cmake_minimum_required( VERSION ${CMAKE_VERSION} )

project( ${PROJECT} LANGUAGES CXX )
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_EXTENSIONS OFF )
set( CMAKE_VERSION 3.2 )
set( CMAKE_MODULE_PATH ${${PROJECT}_SOURCE_DIR}/.cmake )

include( LibPackage )

package_git( ${PROJECT} )
package_git_submodule( libstdhl   master REQUIRED obj   ../../lib/stdhl   ../stdhl   lib/stdhl   )
package_git_submodule( libpass    master REQUIRED obj   ../../lib/pass    ../pass    lib/pass    )
package_git_submodule( libtptp    master REQUIRED obj   ../../lib/tptp    ../tptp    lib/tptp    )
package_git_submodule( libcasm-ir master REQUIRED obj   ../../lib/casm-ir ../casm-ir lib/casm-ir )
package_git_submodule( libcasm-fe master REQUIRED obj   ../../lib/casm-fe ../casm-fe lib/casm-fe )
package_git_submodule( libcasm-tc master REQUIRED obj   ../../lib/casm-tc ../casm-tc lib/casm-tc )
package_git_submodule( libgtest   master REQUIRED build ../../lib/gtest   ../gtest   lib/gtest   )
package_git_submodule( libhayai   master REQUIRED build ../../lib/hayai   ../hayai   lib/hayai   )
package_git_submodule( libz3      master REQUIRED build ../../lib/z3      ../z3      lib/z3      )
package_git_deps(
  libgtest>libstdhl
  libhayai>libstdhl
  libstdhl>libpass
  libpass>libtptp
  libtptp>libcasm-ir
  libcasm-ir>libcasm-fe
  libcasm-fe>libcasm-tc
  )

find_package( Threads REQUIRED )

include( ECMGenerateHeaders )
include( FeatureSummary )
include( GenerateExportHeader )
include( CPack )

add_subdirectory( src )
add_subdirectory( etc
  EXCLUDE_FROM_ALL
  )


#
# executable
#

add_executable( ${PROJECT}
  $<TARGET_OBJECTS:${PROJECT}-cpp>
  )

if( CMAKE_BUILD_TYPE EQUAL "RELEASE" )
  set( PROJECT_EXE_FLAGS "-static" )
endif()

if( APPLE )
  set( PROJECT_EXE_FLAGS "" )
endif()

if( ${LIBCASM_FE_FOUND}
AND ${LIBCASM_IR_FOUND}
AND ${LIBZ3_FOUND}
AND ${LIBTPTP_FOUND}
AND ${LIBPASS_FOUND}
AND ${LIBSTDHL_FOUND}
)
  target_link_libraries( ${PROJECT} PUBLIC ${PROJECT_EXE_FLAGS}
    ${LIBCASM_FE_LIBRARY}
    ${LIBCASM_IR_LIBRARY}
    ${LIBZ3_LIBRARY}
    ${LIBTPTP_LIBRARY}
    ${LIBPASS_LIBRARY}
    ${LIBSTDHL_LIBRARY}
    Threads::Threads
    )
endif()

set( ${PROJECT}_VERSION ${GIT_REVTAG} )
set_property( TARGET ${PROJECT} PROPERTY VERSION ${${PROJECT}_VERSION} )
set_property( TARGET ${PROJECT} PROPERTY PREFIX  "" )


#
# test
#

add_executable( ${PROJECT}-check
  EXCLUDE_FROM_ALL
  $<TARGET_OBJECTS:${PROJECT}-test>
  )

set( PROJECT_LD_TEST "-Wl,--whole-archive ${LIBCASM_TC_TEST} -Wl,--no-whole-archive" )
if( APPLE )
  set( PROJECT_LD_TEST "-Wl,-force_load ${LIBCASM_TC_TEST}" )
endif()

if( ${LIBCASM_TC_FOUND} )
  target_link_libraries( ${PROJECT}-check
    ${PROJECT_LD_TEST}
    ${LIBPASS_LIBRARY}
    ${LIBSTDHL_LIBRARY}
    ${LIBGTEST_LIBRARY}
    ${LIBGTEST_MAIN}
    Threads::Threads
    )
endif()


#
# benchmark
#

add_executable( ${PROJECT}-run
  EXCLUDE_FROM_ALL
  $<TARGET_OBJECTS:${PROJECT}-benchmark>
  )

set( PROJECT_LD_BENCHMARK "-Wl,--whole-archive ${LIBCASM_TC_BENCHMARK} -Wl,--no-whole-archive" )
if( APPLE )
  set( PROJECT_LD_BENCHMARK "-Wl,-force_load ${LIBCASM_TC_BENCHMARK}" )
endif()

if( ${LIBCASM_TC_FOUND} )
  target_link_libraries( ${PROJECT}-run
    ${PROJECT_LD_BENCHMARK}
    ${LIBPASS_LIBRARY}
    ${LIBSTDHL_LIBRARY}
    ${LIBHAYAI_LIBRARY}
    ${LIBGTEST_LIBRARY}
    Threads::Threads
    )
endif()


#
# install
#

install(
  TARGETS  ${PROJECT}
  EXPORT   ${PROJECT}Targets
  LIBRARY  DESTINATION lib
  ARCHIVE  DESTINATION lib
  RUNTIME  DESTINATION bin
  INCLUDES DESTINATION include
  )


# #
# # cmake
# #

# install(
#   FILES
#     .cmake/Find${PROJECT}.cmake
#   DESTINATION
#     "share/cmake/Module/${PROJECT}"
#   )


#
# build features and checks
#

include( LibCompile )
compile_flags( CXX )
compile_check()

feature_summary( WHAT
  PACKAGES_NOT_FOUND
  DISABLED_FEATURES
  )
