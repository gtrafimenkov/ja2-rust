CLANG_FORMATTER ?= clang-format

.PHONY: format format-modified linux-bin run-linux-bin unittester-bin run-unittester

build: linux-bin unittester-bin

clean:
	rm -rf tmp
	rm -rf Debug
	rm -rf Release
	rm -rf ReleaseWithDebug
	rm -rf ja2lib/Debug
	rm -rf ja2lib/Release
	rm -rf ja2lib/ReleaseWithDebug
	rm -rf bin-win32/Debug
	rm -rf bin-win32/Release
	rm -rf bin-win32/ReleaseWithDebug
	rm -rf unittester/Debug
	rm -rf unittester/Release
	rm -rf rustlib/target

test: run-unittester

format:
	find . \( -iname '*.c' -o -iname '*.cc' -o -iname '*.cpp' -o -iname '*.h' \) \
		| grep -v rust_ | xargs $(CLANG_FORMATTER) -i --style=file

format-modified:
	git status --porcelain | egrep -e '[.](c|cc|cpp|h)$$' | awk '{print $$2}' \
		| grep -v rust_ | xargs $(CLANG_FORMATTER) -i --style=file

###################################################################
# linux build
###################################################################

GCC_ERRORS_FLAGS := -Werror
GCC_ERRORS_FLAGS += -Wall
GCC_FLAGS        := -D__GCC
INCLUDE_FLAGS    := -I./ja2lib -I./rustlib
CC = gcc
CXX	= g++
CFLAG = -fPIC --std=gnu17 $(GCC_ERRORS_FLAGS) $(GCC_FLAGS) -DFORCE_ASSERTS_ON $(INCLUDE_FLAGS)
CXXFLAG = -fPIC --std=gnu++17 $(GCC_ERRORS_FLAGS) $(GCC_FLAGS) -DFORCE_ASSERTS_ON $(INCLUDE_FLAGS)

TARGET_ARCH    ?=
ifeq "$(TARGET_ARCH)" ""
BUILD_DIR      := tmp/default
else
BUILD_DIR      := tmp/$(TARGET_ARCH)
endif

RUSTLIB_PATH       := ./rustlib/target/debug
RUSTLIB_NAME       := rustlib
RUSTLIB_FILENAME   := librustlib.so
RUSTLIB_FILEPATH   := rustlib/target/debug/$(RUSTLIB_FILENAME)

JA2LIB_SOURCES := $(shell find ja2lib -name '*.c')
JA2LIB_OBJS0   := $(filter %.o, $(JA2LIB_SOURCES:.c=.o) $(JA2LIB_SOURCES:.cc=.o) $(JA2LIB_SOURCES:.cpp=.o))
JA2LIB_OBJS    := $(addprefix $(BUILD_DIR)/,$(JA2LIB_OBJS0))

LINUX_BIN_SOURCES      := $(shell find bin-linux -name '*.c')
LINUX_BIN_OBJS0        := $(filter %.o, $(LINUX_BIN_SOURCES:.c=.o) $(LINUX_BIN_SOURCES:.cc=.o) $(LINUX_BIN_SOURCES:.cpp=.o))
LINUX_BIN_OBJS         := $(addprefix $(BUILD_DIR)/,$(LINUX_BIN_OBJS0))

UNITTESTER_SOURCES := $(shell find unittester -name '*.c' -o -name '*.cc' -o -name '*.cpp')
UNITTESTER_OBJS0   := $(filter %.o, $(UNITTESTER_SOURCES:.c=.o) $(UNITTESTER_SOURCES:.cc=.o) $(UNITTESTER_SOURCES:.cpp=.o))
UNITTESTER_OBJS    := $(addprefix $(BUILD_DIR)/,$(UNITTESTER_OBJS0))

DUMMY_PLATFORM_SOURCES := $(shell find platform-dummy -name '*.c')
DUMMY_PLATFORM_OBJS0   := $(filter %.o, $(DUMMY_PLATFORM_SOURCES:.c=.o) $(DUMMY_PLATFORM_SOURCES:.cc=.o) $(DUMMY_PLATFORM_SOURCES:.cpp=.o))
DUMMY_PLATFORM_OBJS    := $(addprefix $(BUILD_DIR)/,$(DUMMY_PLATFORM_OBJS0))

LINUX_PLATFORM_SOURCES := $(shell find platform-linux -name '*.c')
LINUX_PLATFORM_OBJS0   := $(filter %.o, $(LINUX_PLATFORM_SOURCES:.c=.o) $(LINUX_PLATFORM_SOURCES:.cc=.o) $(LINUX_PLATFORM_SOURCES:.cpp=.o))
LINUX_PLATFORM_OBJS    := $(addprefix $(BUILD_DIR)/,$(LINUX_PLATFORM_OBJS0))

LIBS         := -lpthread
# LIBS         += -lgtest

$(BUILD_DIR)/%.o: %.c
	@echo .. compiling $<
	@mkdir -p $$(dirname $@)
	@$(CC) $(CFLAG) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cc
	@echo .. compiling $<
	@mkdir -p $$(dirname $@)
	@$(CXX) $(CXXFLAG) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cpp
	@echo .. compiling $<
	@mkdir -p $$(dirname $@)
	@$(CXX) $(CXXFLAG) -c -o $@ $<

libs: $(BUILD_DIR)/ja2lib.a $(BUILD_DIR)/dummy-platform.a

$(BUILD_DIR)/ja2lib.a: $(JA2LIB_OBJS)
	@echo .. building $@
	@ar rcs $@ $^

$(BUILD_DIR)/dummy-platform.a: $(DUMMY_PLATFORM_OBJS)
	@echo .. building $@
	@ar rcs $@ $^

$(BUILD_DIR)/linux-platform.a: $(LINUX_PLATFORM_OBJS)
	@echo .. building $@
	@ar rcs $@ $^

linux-bin: $(BUILD_DIR)/bin/ja2-linux

$(BUILD_DIR)/bin/ja2-linux: $(LINUX_BIN_OBJS) $(BUILD_DIR)/ja2lib.a $(BUILD_DIR)/linux-platform.a $(BUILD_DIR)/bin/$(RUSTLIB_FILENAME)
	@echo .. building $@
	@mkdir -p $(BUILD_DIR)/bin
	@$(CC) $(CFLAG) $^ -o $@ -lpthread -L$(RUSTLIB_PATH) -lrustlib

run-linux-bin: $(BUILD_DIR)/bin/ja2-linux
	@cp $(BUILD_DIR)/bin/ja2-linux ../ja2-installed
	@echo -----------------------------------------------------------------
	@cd ../ja2-installed && ./ja2-linux

unittester-bin: $(BUILD_DIR)/bin/unittester

$(RUSTLIB_FILEPATH):
	make -C rustlib

$(BUILD_DIR)/bin/$(RUSTLIB_FILENAME): $(RUSTLIB_FILEPATH)
	mkdir -p $(BUILD_DIR)/bin
	cp $(RUSTLIB_FILEPATH) $@

$(BUILD_DIR)/bin/unittester: $(UNITTESTER_OBJS) $(BUILD_DIR)/ja2lib.a $(BUILD_DIR)/linux-platform.a $(BUILD_DIR)/bin/$(RUSTLIB_FILENAME)
	@echo .. building $@
	@mkdir -p $(BUILD_DIR)/bin
	@$(CXX) $(CFLAG) $^ -o $@ -lgtest_main -lgtest -lpthread -L$(RUSTLIB_PATH) -lrustlib

run-unittester: $(BUILD_DIR)/bin/unittester
	./$(BUILD_DIR)/bin/unittester

###################################################################
#
###################################################################

install-build-dependencies-deb:
	sudo apt install -y libgtest-dev

install-build-dependencies-yum:
	sudo yum install -y gtest-devel

install-dev-dependencies-deb:
	sudo apt install clang-format-13 libgtest-dev
