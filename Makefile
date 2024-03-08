CXX := clang++
CXX_FLAGS := $(CXX_FLAGS) -Wall -std=c++11
CXX_LIBS := -pthread

EXECUTABLE_NAME := test

TARGET_DIR := target
BUILD_DIR := build
SRC_DIR := src

# Directories with platform-dependent code
# This dirs should contain directories with the name
# of the platform
PLATFORM_DEPENDENT_DIRS :=

# SOURCES := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
SOURCES := $(shell find $(SRC_DIR) -follow -name '*.cpp')


ifeq ($(OS), Windows_NT)
	PLATFORM := WINDOWS
	PLATFORM_LC := windows
else
	PLATFORM := UNIX
	PLATFORM_LC := unix
endif

IGNORED_SOURCES := $(shell find $(PLATFORM_DEPENDENT_DIRS) -name '*.cpp')
PLATFORM_DEPS := $(shell find $(addsuffix /$(PLATFORM_LC), $(PLATFORM_DEPENDENT_DIRS)) -name '*.cpp')
IGNORED_SOURCES := $(filter-out $(PLATFORM_DEPS), $(IGNORED_SOURCES))

SOURCES := $(filter-out $(IGNORED_SOURCES), $(SOURCES))
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

EXECUTABLE := $(TARGET_DIR)/$(EXECUTABLE_NAME)
EXECUTABLE_SOURCE := $(SRC_DIR)/test.cpp
EXECUTABLE_OBJ := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(EXECUTABLE_SOURCE))

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(CXX_LIBS)

$(EXECUTABLE_OBJ): $(EXECUTABLE_SOURCE)
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.cpp src/%.h
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

.PHONY: release
release: CXX_FLAGS := $(CXX_FLAGS) -O3
release: $(EXECUTABLE)


.PHONY: clean
clean:
	rm -r $(BUILD_DIR) $(TARGET_DIR)
