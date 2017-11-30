CXX=g++
CFLAGS=-g -Wall
LFLAGS=
TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	rm -rf $(TARGET)
	rm -rf *.dSYM
