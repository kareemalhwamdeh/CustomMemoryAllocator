CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g

SRC = allocator.c main.c
OBJ = $(SRC:.c=.o)
TARGET = memory_allocator

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
