CC		= gcc
CFLAGS	= -std=c2x -Wall -Wextra -Wpedantic -g

PKG_CFLAGS	= $(shell pkg-config --cflags sdl3 sdl3-ttf)
# -lm links the math library (needed for sin/cos/fmodf used in menu animation)
PKG_LIBS	= $(shell pkg-config --libs sdl3 sdl3-ttf) -lm

TARGET = tictactoe
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(PKG_CFLAGS) -o $(TARGET) $(SRC) $(PKG_LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all run clean
