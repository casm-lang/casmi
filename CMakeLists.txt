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

set( PROJECT casmi )

cmake_minimum_required( VERSION ${CMAKE_VERSION} )

project( ${PROJECT} LANGUAGES C CXX )
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_VERSION 3.2 )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_MODULE_PATH ${${PROJECT}_SOURCE_DIR}/.cmake )

include( LibPackage )

package_git( ${PROJECT} )
package_git_submodule( libstdhl   master REQUIRED obj   ../../lib/stdhl   lib/stdhl   )
package_git_submodule( libpass    master REQUIRED obj   ../../lib/pass    lib/pass    )
package_git_submodule( libcasm-ir master REQUIRED obj   ../../lib/casm-ir lib/casm-ir )
package_git_submodule( libcasm-rt master REQUIRED obj   ../../lib/casm-rt lib/casm-rt )
package_git_submodule( libcasm-fe master REQUIRED obj   ../../lib/casm-fe lib/casm-fe )
package_git_submodule( libcasm-tc master REQUIRED obj   ../../lib/casm-tc lib/casm-tc )
package_git_submodule( libgtest   master REQUIRED build ../../lib/gtest   lib/gtest   )
package_git_submodule( libhayai   master REQUIRED build ../../lib/hayai   lib/hayai   )

find_package( Threads REQUIRED )

include( ECMGenerateHeaders )
include( FeatureSummary )
include( GenerateExportHeader )
include( CPack )

add_subdirectory( src )
#add_subdirectory( etc
#  EXCLUDE_FROM_ALL
#  )

add_custom_target( sync
  COMMAND
    make sync > ${PROJECT_BINARY_DIR}/CMakeSync 2>&1
  DEPENDS
    libgtest
    libhayai
    libstdhl
    libpass
    libcasm-ir
    libcasm-rt
    libcasm-fe
  WORKING_DIRECTORY
    ${PROJECT_SOURCE_DIR}
  )

add_dependencies( ${PROJECT}-cpp
  sync
  )


#
# executable
#

add_executable( ${PROJECT}
  $<TARGET_OBJECTS:${PROJECT}-cpp>
  )

if( ${LIBCASM_FE_FOUND} )
  target_link_libraries( ${PROJECT}
    ${LIBCASM_FE_LIBRARY}
    ${LIBCASM_RT_LIBRARY}
    ${LIBCASM_IR_LIBRARY}
    ${LIBPASS_LIBRARY}
    ${LIBSTDHL_LIBRARY}
    Threads::Threads
    )
endif()


set( ${PROJECT}_VERSION ${GIT_REVTAG} )
set_property( TARGET ${PROJECT} PROPERTY VERSION   ${PROJECT}_VERSION )
set_property( TARGET ${PROJECT} PROPERTY PREFIX "" )

install(
  TARGETS  ${PROJECT}
  EXPORT   ${PROJECT}Targets
  LIBRARY  DESTINATION lib
  ARCHIVE  DESTINATION lib
  RUNTIME  DESTINATION bin
  INCLUDES DESTINATION include
  )


# #
# # test
# # 
# 
# 
# add_executable( ${PROJECT}-check
#   EXCLUDE_FROM_ALL
#   $<TARGET_OBJECTS:${PROJECT}-test>
#   )
# 
# if( ${LIBGTEST_FOUND} )
#   target_link_libraries( ${PROJECT}-check
#     ${PROJECT}
#     ${LIBGTEST_LIBRARY}
#     ${LIBGTEST_MAIN}
#     Threads::Threads
#     )
# endif()
# 
# #
# # benchmark
# #
# 
# add_executable( ${PROJECT}-run
#   EXCLUDE_FROM_ALL
#   $<TARGET_OBJECTS:${PROJECT}-benchmark>
#   )
# 
# if( ${LIBHAYAI_FOUND} )
#   target_link_libraries( ${PROJECT}-run
#     ${PROJECT}
#     ${LIBHAYAI_LIBRARY}
#     )
# endif()
# 
# 
# #
# # cmake
# #
# 
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
feature_summary( WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES )
