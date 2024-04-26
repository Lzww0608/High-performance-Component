
CXX=g++

SRC=main.cpp

TARGET=main

CXXFLAGS=-std=c++14 -lpthread -I/usr/local/include

all: $(TARGET)
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS)

clean:
	rm -f $(TARGET)