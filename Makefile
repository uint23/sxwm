CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -Isrc
LDFLAGS = -lX11

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:.c=.o)
BIN = sxwm

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

x:
	rm -f $(SRC_DIR)/*.o $(BIN)

r:
	x all

.PHONY: all clean rebuild
