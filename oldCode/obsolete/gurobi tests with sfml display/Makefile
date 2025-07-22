# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++11 -pthread -m64
# Include directory for Gurobi header files
INCLUDES = -I/home/sebas/gurobi1102/linux64/include
# Library directory for Gurobi libraries
LIBDIR = -L/home/sebas/gurobi1102/linux64/lib
# Libraries to link against
LIBS = -lgurobi_c++ -lgurobi110 -lsfml-graphics -lsfml-window -lsfml-system -lX11

# Source files
SRCS = main.cpp graphs.cpp modelWrapper.cpp SFMLGraphs.cpp
# Object files
OBJS = $(SRCS:.cpp=.o)
# Executable name
TARGET = gurobiSFML_test

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
