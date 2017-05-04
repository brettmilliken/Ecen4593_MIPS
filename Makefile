CSIGN = -std=c++11
CC = g++
CFLAGS = -Wall -Wextra -g
RM = rm 
all: mips 

mips: main.o Load_Program.o defstage.o
	$(CC) $(CFLAGS) -o mips main.o Load_Program.o defstage.o

main.o: main.cpp pc.hpp Load_Program.hpp defstage.hpp
	$(CC) $(CFLAGS) -c main.cpp

Load_Program.o: Load_Program.cpp Load_Program.hpp
	$(CC) $(CFLAGS) -c Load_Program.cpp

defstage.o: defstage.cpp stagereg.hpp pc.hpp defstage.hpp
	$(CC) $(CFLAGS) $(CSIGN) -c defstage.cpp

clean:
	$(RM) mips *.o
