#
#   Copyright (C) 2014-2022 CASM Organization <https://casm-lang.org>
#   All rights reserved.
#
#   Developed by: Philipp Paulweber et al.
#                 <https://github.com/casm-lang/casmi/graphs/contributors>
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

include_directories(
  ${PROJECT_BINARY_DIR}/src
  ${LIBSTDHL_INCLUDE_DIR}
  ${LIBPASS_INCLUDE_DIR}
  ${LIBTPTP_INCLUDE_DIR}
  ${LIBZ3_INCLUDE_DIR}
  ${LIBCASM_IR_INCLUDE_DIR}
  ${LIBCASM_FE_INCLUDE_DIR}
  ${LIBCASM_TC_INCLUDE_DIR}
  )

add_library( ${PROJECT}-cpp OBJECT
  casmi.cpp
  )

configure_file(
  Version.in.h
  ${PROJECT}/Version.h
  )
configure_file(
  Version.in
  ${PROJECT}/Version
  )


# ecm_generate_headers( ${PROJECT}_HEADERS_CPP
#   ORIGINAL
#     CAMELCASE
#   HEADER_NAMES
#     HeaderNameHere!
#     Value
#   PREFIX
#     ${PROJECT}
#   REQUIRED_HEADERS
#     ${PROJECT}_HEADERS
#   )
# install(
#   FILES
#     ${${PROJECT}_HEADERS}
#     ${${PROJECT}_HEADERS_CPP}
#     ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}/Version
#   DESTINATION
#     "include/${PROJECT}"
#   )


# ecm_generate_headers( ${PROJECT}_SUBDIR_HEADERS_CPP
#   ORIGINAL
#     CAMELCASE
#   HEADER_NAMES
#     # FILE
#   PREFIX
#     ${PROJECT}/subdir
#   RELATIVE
#     subdir
#   REQUIRED_HEADERS
#     ${PROJECT}_SUBDIR_HEADERS
#   )
# install(
#   FILES
#     ${${PROJECT}_SUBDIR_HEADERS}
#     ${${PROJECT}_SUBDIR_HEADERS_CPP}
#   DESTINATION
#     "include/${PROJECT}/subdir"
#   )
