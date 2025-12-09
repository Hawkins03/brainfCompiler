
CC = gcc
CFLAGS = -Wall -g


TARGET = brainfCompiler

SOURCES = main.c bf_interp.c ms_parser.c utils.c
DEPS = bf.h ms.h utils.h

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
