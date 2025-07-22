# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++11 -pthread -m64
# Include directory for Gurobi header files
INCLUDES = -I/home/sebas/gurobi1102/linux64/include -Iinclude
# Library directory for Gurobi libraries
LIBDIR = -L/home/sebas/gurobi1102/linux64/lib
# Libraries to link against
LIBS = -lgurobi_c++ -lgurobi110 -lsfml-graphics -lsfml-window -lsfml-system -lX11


# Source files
SRCS = $(wildcard src/*.cpp) 
# Object files
OBJS = $(SRCS:src/%.cpp=build/obj/%.o)
# Executable name
TARGET = build/bin/exec

# Default rule to build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBDIR) -o $@ $^ $(LIBS)

# Rule to compile source files into object files
build/obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# Clean rule to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
