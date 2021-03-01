# .RECIPEPREFIX = 

CXX      := g++
CXXFLAGS := -pedantic-errors -Wall -Wextra
LDFLAGS  := -lstdc++fs
BUILD    := ./build
OBJ_DIR  := $(BUILD)
TARGET   := $(notdir $(CURDIR))
INCLUDE  := -Iinclude/
SRC      := $(wildcard source/*.cpp)

OBJECTS  := $(SRC:%.cpp=$(BUILD)/%.o)
DEPENDENCIES \
         := $(OBJECTS:.o=.d)

all: build $(TARGET)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean

build:
	@mkdir -p $(BUILD)

clean:
	-@rm -rvf $(BUILD)