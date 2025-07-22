# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++14 -pthread -m64 -g 
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
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJS:%.o=%.d)


all: $(TARGET)

# Default rule to build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBDIR) -o $@ $^ $(LIBS)

-include $(DEP)

# Rule to compile source files into object files
build/obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -MMD $(INCLUDES) -c -o $@ $<


# Clean rule to remove object files and executable
clean:
	rm -f $(DEP) $(OBJS) $(TARGET) 

# Phony targets
.PHONY: all clean
