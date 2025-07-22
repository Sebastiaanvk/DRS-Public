# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++11 -pthread -m64
# Include directory for Gurobi header files
INCLUDES = -I/home/sebas/gurobi1102/linux64/include
# Library directory for Gurobi libraries
LIBDIR = -L/home/sebas/gurobi1102/linux64/lib
# Libraries to link against
LIBS = -lgurobi_c++ -lgurobi110

# Source files
SRCS = main.cpp
# Object files
OBJS = $(SRCS:.cpp=.o)
# Executable name
TARGET = my_program

# Default rule to build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBDIR) -o $@ $^ $(LIBS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# Clean rule to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
