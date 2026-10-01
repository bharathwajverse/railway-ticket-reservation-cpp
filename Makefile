# Compiler and Flags
CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -IDSA/include -Idatabase/include

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

# Application modules
SRCS = app/main.cpp DSA/src/dsa_manager.cpp database/src/database.cpp

# Default target
all: $(TARGET)

# Link executable

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)
	@echo "Build successful: $(TARGET)"

# Run application
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
ifeq ($(OS),Windows_NT)
	-del /Q /F $(TARGET) 2>nul
else
	$(RM) $(TARGET)
endif
