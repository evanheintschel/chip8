OBJS = main.o chip8.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

chip8 : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o chip8

main.o : main.cpp chip8.h
	$(CC) $(CFLAGS) main.cpp

chip8.o : chip8.h chip8.cpp
	$(CC) $(CFLAGS) chip8.cpp

clean :
	\rm *.o *~ chip8

tar :
	tar cfv chip8.tar main.cpp chip8.h chip8.cpp
