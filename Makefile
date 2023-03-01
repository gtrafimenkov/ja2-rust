CLANG_FORMATTER ?= clang-format-13

.PHONY: format format-modified linux-bin run-linux-bin unittester-bin run-unittester

build: linux-bin unittester-bin

format:
	find . \( -iname '*.c' -o -iname '*.cc' -o -iname '*.cpp' -o -iname '*.h' \) \
		| xargs $(CLANG_FORMATTER) -i --style=file

format-modified:
	git status --porcelain | egrep -e '[.](c|cc|cpp|h)$$' | awk '{print $$2}' \
		| xargs $(CLANG_FORMATTER) -i --style=file

###################################################################
# linux build
###################################################################

CC = gcc
CXX	= g++
CFLAG = -fPIC --std=gnu17 -DFORCE_ASSERTS_ON -I./ja2lib
CXXFLAG = -fPIC --std=gnu++17 -DFORCE_ASSERTS_ON -I./ja2lib
# COMPILE_FLAGS = -c -Wall -Werror -DFORCE_ASSERTS_ON -I./ja2lib
# COMPILE_FLAGS = -c -Wall --std=c17 -DFORCE_ASSERTS_ON -I./ja2lib

TARGET_ARCH    ?=
ifeq "$(TARGET_ARCH)" ""
BUILD_DIR      := tmp/default
else
BUILD_DIR      := tmp/$(TARGET_ARCH)
endif

# JA2LIB_SOURCES := $(shell grep -l "// build:linux" -r ja2lib)
JA2LIB_SOURCES := $(shell find ja2lib -name '*.c')
JA2LIB_OBJS0   := $(filter %.o, $(JA2LIB_SOURCES:.c=.o) $(JA2LIB_SOURCES:.cc=.o) $(JA2LIB_SOURCES:.cpp=.o))
JA2LIB_OBJS    := $(addprefix $(BUILD_DIR)/,$(JA2LIB_OBJS0))

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
	@$(CC) $(CFLAG) -c $(COMPILE_FLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.cc
	@echo .. compiling $<
	@mkdir -p $$(dirname $@)
	@$(CXX) $(CXXFLAG) -c $(COMPILE_FLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.cpp
	@echo .. compiling $<
	@mkdir -p $$(dirname $@)
	@$(CXX) $(CXXFLAG) -c $(COMPILE_FLAGS) -o $@ $<

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

$(BUILD_DIR)/bin/ja2-linux: bin-linux/main.c $(BUILD_DIR)/linux-platform.a $(BUILD_DIR)/ja2lib.a
	@echo .. building $@
	@mkdir -p $(BUILD_DIR)/bin
	@$(CC) $(CFLAG) $(COMPILE_FLAGS) $^ -o $@

run-linux-bin: $(BUILD_DIR)/bin/ja2-linux
	@cp $(BUILD_DIR)/bin/ja2-linux ../ja2-installed
	@echo -----------------------------------------------------------------
	@cd ../ja2-installed && ./ja2-linux

unittester-bin: $(BUILD_DIR)/bin/unittester

$(BUILD_DIR)/bin/unittester: $(UNITTESTER_OBJS) $(BUILD_DIR)/ja2lib.a $(BUILD_DIR)/linux-platform.a
	@echo .. building $@
	@mkdir -p $(BUILD_DIR)/bin
	@$(CXX) $(CFLAG) $(COMPILE_FLAGS) $^ -o $@ -lgtest_main -lgtest -lpthread

run-unittester: $(BUILD_DIR)/bin/unittester
	./$(BUILD_DIR)/bin/unittester

###################################################################
#
###################################################################

install-build-dependencies-deb:
	sudo apt install -y libgtest-dev

install-dev-dependencies-deb:
	sudo apt install clang-format-13 libgtest-dev
