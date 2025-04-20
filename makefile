# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O2 -g -fopenmp
INCLUDES = -I parlaylib/include

# Target and source
TARGET = final
SRC = main.cpp

all: final

# Build rule
$(TARGET): $(SRC)	
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRC)

relabel: relabel.cpp
	$(CXX) $(CXXFLAGSs) -o relabel relabel.cpp

# Clean rule
clean:
	rm -f $(TARGET) relabel