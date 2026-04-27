CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Ofast -march=native -Iinclude $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lGL -lm -lSDL2_ttf
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
TARGET = program

all: $(TARGET)

# Rule for debugging
.PHONY: debug
debug: CFLAGS += -g

debug: clean $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Link object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile each .c file into build/*.o
build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Remove build folder and executable
clean:
	rm -rf build $(TARGET)

-include $(OBJ:.o=.d)
