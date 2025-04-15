CC = gcc
CFLAGS = -Wall -Wextra -O3 -g -Isrc
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

c:
	rm -f $(SRC_DIR)/*.o $(BIN)

.PHONY: all c
