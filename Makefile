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

.PHONY: obj/version.h obj/license.h

default: debug

help:
	@echo "TODO"


TARGET=casmi

CP  = $(shell find src -name '*.cpp' | cut -d'.' -f1)
CO  = $(CP:%=obj/%.o)

CI  = -I src
CI += -I src/ir
CI += -I obj
CI += -I lib
CI += -I lib/casm-fe
CI += -I lib/casm-ir
CI += -I lib/casm-tc
CI += -I lib/pass

CL  = obj/casmi.a
CL += lib/pass/libpass.a
CL += lib/stdhl/libstdhlc.a
CL += lib/stdhl/libstdhlcpp.a
CL += lib/casm-fe/libcasm-fe.a
CL += lib/casm-ir/libcasm-ir.a

CC  =
CF  =

  %-gcc: CC = gcc
%-clang: CC = clang

  debug-%: CF += -O0 -g
release-%: CF += -O3 -DNDEBUG

linux%:  CF += -Wall -std=c++11
linux%:  XF += -Wall -std=c11
linux3%: CF += -m32
linux6%: CF += -m64


build: config $(TARGET)
check: build $(TEST_TARGET)

linux32-build: build
linux64-build: build

linux32-check: check
linux64-check: check


  debug-build-linux32-gcc:   linux32-build
  debug-check-linux32-gcc:   linux32-check
release-build-linux32-gcc:   linux32-build
release-check-linux32-gcc:   linux32-check

  debug-build-linux64-gcc:   linux64-build
  debug-check-linux64-gcc:   linux64-check
release-build-linux64-gcc:   linux64-build
release-check-linux64-gcc:   linux64-check

  debug-build-linux32-clang: linux32-build
  debug-check-linux32-clang: linux32-check
release-build-linux32-clang: linux32-build
release-check-linux32-clang: linux32-check

  debug-build-linux64-clang: linux64-build
  debug-check-linux64-clang: linux64-check
release-build-linux64-clang: linux64-build
release-check-linux64-clang: linux64-check


  debug:   debug-build-linux64-clang
release: clean release-build-linux64-clang

test:           debug-check-linux64-clang
test-release: release-check-linux64-clang


all: clean default

#doxy:
#	doxygen

config: CFG="CC=$(CC) CF=\"$(CF)\""
config:
	@echo "CFG  $(CFG)"


obj/%.o: %.cpp
	@mkdir -p `dirname $@`
	@echo "C++ " $<
	@$(CC) $(CF) $(CI) -c $< -o $@

obj/%.o: %.c
	@mkdir -p `dirname $@`
	@echo "C   " $<
	@$(CC) $(CF) $(CI) -c $< -o $@



lib/pass/libpass.a: lib/pass
	@cd $<; $(MAKE) build CC="$(CC)" CF="$(CF)"

lib/stdhl/libstdhlcpp.a: lib/stdhl
	@cd $<; $(MAKE) build CC="$(CC)" CF="$(CF)"

lib/casm-fe/libcasm-fe.a: lib/casm-fe
	@cd $<; $(MAKE) build CC="$(CC)" CF="$(CF)"

lib/casm-ir/libcasm-ir.a: lib/casm-ir
	@cd $<; $(MAKE) build CC="$(CC)" CF="$(CF)"


obj/version.h:
	@mkdir -p `dirname $@`
	@echo "GEN " $@ 
	@echo "#define VERSION \""`git describe --always --tags --dirty`"\"" > $@

obj/license.h:
	@mkdir -p `dirname $@`
	@echo "GEN " $@
	echo "#define LICENSE \"TBD\"" > $@

obj/casmi.a: $(CO)
	@echo "AR  " $@
	@$(AR) rsc $@ $(filter %.o,$^)
	@ranlib $@


$(TARGET): obj/version.h obj/license.h $(CL)
	@echo "LD  " $@
	@$(CC) $(CF) -o $@ $(filter %.o,$^) $(filter %.a,$^) -lstdc++ -lm

clean:
	@echo "RMD " obj
	@rm -rf obj
	@echo "RM  " $(TARGET)
	@rm -f $(TARGET)
	$(MAKE) clean -C lib/casm-fe
	$(MAKE) clean -C lib/casm-ir
	@rm -f test
	@rm -f $(TEST_TARGET)


TEST_TARGET = obj/casmi-test.a

TEST_FILES   = $(shell find uts -name '*.cpp' | cut -d'.' -f1)
TEST_OBJECTS = $(TEST_FILES:%=obj/%.o)

TEST_INCLUDE  = -I lib/gtest/googletest/include
TEST_INCLUDE += -I lib/gtest/googletest

TEST_LIBRARY  = -lstdc++
TEST_LIBRARY += -lm
TEST_LIBRARY += -lpthread

obj/uts/%.o: uts/%.cpp
	@mkdir -p `dirname $@`
	@echo "C++ " $<
	@$(CC) $(CF) $(TEST_INCLUDE) $(CI) -c $< -o $@


$(TEST_TARGET): $(TEST_OBJECTS)
	@echo "AR  " $@
	@$(AR) rsc $@ $(filter %.o,$^)
	@ranlib $@

lib/casm-fe/libcasm-fe-test.a: lib/casm-fe
	@cd $<; $(MAKE) libcasm-fe-test.a

lib/casm-ir/libcasm-ir-test.a: lib/casm-ir
	@cd $<; $(MAKE) libcasm-ir-test.a

test: $(CL:%.a=%-test.a)
	@echo "LD  " $@
	@$(CC) $(CF) $(TEST_INCLUDE) -o $@ $^ $(TEST_LIBRARY) \
		 lib/gtest/googletest/src/gtest-all.cc lib/gtest/googletest/src/gtest_main.cc 
	@echo "RUN " $@
	@./$@

# test: $(TARGET) $(TEST_TARGET) default
# 	@rm -f $@
# 	@echo "LD  " $@
# 	@$(CC) $(CF) $(TEST_INCLUDE) $(CI) $(TEST_LIBRARY) -o $@ \
# 		-Wl,--whole-archive $(TEST_TARGET) $(TARGET) -Wl,--no-whole-archive \
# 		 ../gtest/googletest/src/gtest-all.cc ../gtest/googletest/src/gtest_main.cc 
# 	@echo "RUN " $@
# 	@./$@
