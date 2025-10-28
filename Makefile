CXX = g++
CXXFLAGS = -std=c++20 

SRC = server.cpp

TARGET = kvstore

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
