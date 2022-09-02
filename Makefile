TARGET = ./particles

CC = clang
CFLAGS = `sdl2-config --cflags` -O3 -Wall -flto
LDFLAGS = `sdl2-config --libs` -lGL -lm -flto

.PHONY: default all run clean

default: $(TARGET)
all: default run

SRC_C =  $(wildcard *.c)

OBJECTS = $(SRC_C:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

run: $(TARGET)
	$(TARGET) 

clean:
	-rm -f *.o
	-rm -f $(TARGET)
