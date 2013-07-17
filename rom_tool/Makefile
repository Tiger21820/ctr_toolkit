OBJS = utils.o main.o ncsd.o
LIBS = -static-libgcc -static-libstdc++
CXXFLAGS = -I.
CFLAGS = --std=c99 -Wall -I.
OUTPUT = rom_tool
CC = gcc

main: $(OBJS) $(POLAR_OBJS)
	g++ -o $(OUTPUT) $(LIBS) $(OBJS)


clean:
	rm -rf $(OUTPUT) $(OBJS) $(POLAR_OBJS)