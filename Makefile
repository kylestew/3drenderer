CC = gcc
CFLAGS = -Wall -std=c99 -I/opt/homebrew/opt/sdl2/include
LDFLAGS = -L/opt/homebrew/opt/sdl2/lib -lSDL2 -lm
SRC = ./src/*.c
OUT = renderer

build:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(OUT)

run: build
	./$(OUT)

clean:
	rm -f $(OUT)
