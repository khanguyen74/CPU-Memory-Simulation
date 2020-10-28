CFLAGS = -O
CC = g++

main: project1.o CPU.o Memory.o
	$(CC) $(CFLAGS) -o main project1.o CPU.o Memory.o

project1.o: project1.cpp
	$(CC) $(CFLAGS) -c project1.cpp

CPU.o: CPU.cpp
	$(CC) $(CFLAGS) -c CPU.cpp
	
Memory.o: Memory.cpp
	$(CC) $(CFLAGS) -c Memory.cpp

clean: 
	rm -f core *.o
