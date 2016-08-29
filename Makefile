#   
#   Copyright (c) 2014-2016 CASM Organization
#   All rights reserved.
#   
#   Developed by: Florian Hahn
#                 Philipp Paulweber
#                 Emmanuel Pescosta
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

CPP=clang

CPPFLAG += -std=c++11
CPPFLAG += -g -O0
CPPFLAG += -Wall
#CPPFLAG += -Wextra

TARGET=casmi

OBJECTS += obj/casmi.o
OBJECTS += obj/PassManager.o


INCLUDE += -I src
INCLUDE += -I src/ir
INCLUDE += -I obj
INCLUDE += -I lib/casm-fe
INCLUDE += -I lib/casm-ir
INCLUDE += -I lib/casm-tc
INCLUDE += -I lib/pass

INCLUDE += -I lib

LIBRARY  = lib/stdhl/libstdhlc.a
LIBRARY += lib/stdhl/libstdhlcpp.a
LIBRARY += lib/casm-fe/libcasm-fe.a
LIBRARY += lib/casm-ir/libcasm-ir.a


.PHONY: obj/version.h obj/license.h


default: $(TARGET)


all: clean default
#all: clean doxy default

#doxy:
#	doxygen

obj:
	@echo "MKD " obj
	@mkdir -p obj

obj/%.o: src/%.cpp
	@echo "CPP " $<
	@$(CPP) $(CPPFLAG) $(INCLUDE) -c $< -o $@

obj/%.o: src/%.c
	@echo "CC  " $<
	@$(CPP) $(CPPFLAG) $(INCLUDE) -c $< -o $@



lib/stdhl/libstdhlc.a lib/stdhl/libstdhlcpp.a: lib/stdhl
	@cd $<; $(MAKE)

lib/casm-fe/libcasm-fe.a: lib/casm-fe
	@cd $<; $(MAKE)

lib/casm-ir/libcasm-ir.a: lib/casm-ir
	@cd $<; $(MAKE)

obj/version.h: obj
	@echo "GEN " $@ 
	@echo "#define VERSION \""`git describe --always --tags --dirty`"\"" > $@

obj/license.h: obj
	@echo "GEN " $@
	echo "#define LICENSE \"TBD\"" > $@

$(TARGET): obj/version.h obj/license.h $(LIBRARY) $(OBJECTS)
	@echo "LD  " $@
	@$(CPP) $(CPPFLAG) -o $@ $(filter %.o,$^) $(filter %.a,$^) -lstdc++ -lm

clean:
	@echo "RMD " obj
	@rm -rf obj
	@echo "RM  " $(TARGET)
	@rm -f $(TARGET)
	$(MAKE) clean -C lib/casm-fe
	$(MAKE) clean -C lib/casm-ir
	@rm -f test

TEST_FILES   = $(shell find uts -name '*.cpp' | cut -d'.' -f1)
TEST_OBJECTS = $(TEST_FILES:%=obj/%.o)

TEST_INCLUDE  = -I lib/gtest/googletest/include
TEST_INCLUDE += -I lib/gtest/googletest

TEST_LIBRARY  = -lstdc++
TEST_LIBRARY += -lm
TEST_LIBRARY += -lpthread

obj/uts/%.o: uts/%.cpp
	@mkdir -p `dirname $@`
	@echo "CPP " $<
	@$(CPP) $(CPPFLAG) $(TEST_INCLUDE) $(INCLUDE) -c $< -o $@

test: default obj $(TEST_OBJECTS)
	@rm -f $@
	@echo "LD  " $@
#@$(CPP) $(CPPFLAG) $(TEST_INCLUDE) $(INCLUDE) $(TEST_LIBRARY) -o $@ $(filter %.o,$^) $(TARGET) lib/gtest/googletest/src/gtest-all.cc lib/gtest/googletest/src/gtest_main.cc
	@$(CPP) $(CPPFLAG) $(TEST_INCLUDE) $(INCLUDE) $(TEST_LIBRARY) -o $@ $(filter %.o,$^) lib/gtest/googletest/src/gtest-all.cc lib/gtest/googletest/src/gtest_main.cc
	@echo "RUN " $@
	@./$@
