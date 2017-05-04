CSIGN = -std=c++11
CC = g++
CFLAGS = -Wall -Wextra -g
RM = rm 
all: mips 

mips: main.o Load_Program.o defstage.o SA_cache.o main_memory.o
	$(CC) $(CFLAGS) -o mips main.o Load_Program.o defstage.o SA_cache.o

main.o: main.cpp pc.hpp Load_Program.hpp defstage.hpp
	$(CC) $(CFLAGS) -c main.cpp

Load_Program.o: Load_Program.cpp Load_Program.hpp main_memory.hpp
	$(CC) $(CFLAGS) -c Load_Program.cpp

defstage.o: defstage.cpp stagereg.hpp pc.hpp defstage.hpp main_memory.hpp SA_cache.hpp
	$(CC) $(CFLAGS) $(CSIGN) -c defstage.cpp

SA_cache.o: SA_cache.cpp main_memory.hpp SA_cache.hpp
	$(CC) $(CFLAGS) $(CSIGN) -c SA_cache.cpp

main_memory.o: main_memory.cpp main_memory.hpp
	$(CC) $(CFLAGS) -c main_memory.cpp

clean:
	$(RM) mips *.o
