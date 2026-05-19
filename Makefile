CC = gcc
CFLAGS = -Iinclude -Wall -g
LDFLAGS = -lglfw3 -lglew32 -lopengl32

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)

all: build/Project

build/Project: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q build\*.o
	del /Q build\Project.exe