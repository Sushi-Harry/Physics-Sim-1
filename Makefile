# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Iinclude -Wall -Wextra -std=c++11

# Linker flags
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11

# Source files (change as needed)
SRC = $(wildcard *.cpp) $(wildcard imgui/*.cpp)

# Object files (replace .cpp with .o)
OBJ = $(SRC:.cpp=.o)

# Output executable
TARGET = main

# Build rules
all: $(TARGET)

# Rule to link object files and create the final executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Rule to compile each .cpp file into a .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove object files and the executable
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
