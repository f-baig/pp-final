CXX = g++
CXXFLAGS = -std=c++17 -O2 -fopenmp
TARGET = estimate_arboricity
SRC = estimate_arboricity.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
