CC = g++
CXXFLAGS = -Wall -Wextra -g -std=c++11

SRC = $(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
TARGET=mips

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.cpp %.hpp
	$(CC) $(CXXFLAGS) -c $<

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)
