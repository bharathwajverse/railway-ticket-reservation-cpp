# Compiler and Flags
CXX ?= g++
CC ?= gcc
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
CFLAGS = -O2

# Detect OS for libraries and binary name
ifeq ($(OS),Windows_NT)
    TARGET = railway.exe
    LDFLAGS = 
    RM = del /Q /F
else
    TARGET = railway
    LDFLAGS = -lpthread -ldl
    RM = rm -f
endif

# Source Files & Objects
SRCS = main.cpp utils.cpp database.cpp railway.cpp
OBJS = $(SRCS:.cpp=.o) sqlite3.o

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(TARGET)
	@echo "Build successful: $(TARGET)"

# Compile C++ files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile SQLite C amalgamation
sqlite3.o: sqlite3.c sqlite3.h
	$(CC) $(CFLAGS) -c sqlite3.c -o sqlite3.o

# Run demo
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	$(RM) *.o $(TARGET)
