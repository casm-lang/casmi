#
#   Copyright (C) 2014-2020 CASM Organization <https://casm-lang.org>
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

TARGET = casmi

FORMAT  = src
FORMAT += src/*
FORMAT += etc
FORMAT += etc/*
FORMAT += etc/*/*

UPDATE_ROOT = ../../lib/stdhl

include .cmake/config.mk

ENV_FLAGS  = CASM_ARG_PRE=--ast-exec
ENV_FLAGS += CASM="$(OBJ)/$(TARGET)"
