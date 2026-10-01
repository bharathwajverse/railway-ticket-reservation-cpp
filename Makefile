# Compiler and Flags
CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Detect OS for binary name and clean command
ifeq ($(OS),Windows_NT)
    TARGET = railway.exe
    LDFLAGS =
    RM = del /Q /F
else
    TARGET = railway
    LDFLAGS =
    RM = rm -f
endif

# Source Files & Objects
SRCS = main.cpp dsa_manager.cpp database.cpp
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(TARGET)
	@echo "Build successful: $(TARGET)"

# Compile C++ files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run application
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	$(RM) *.o $(TARGET)
