CC		?= gcc
CFLAGS	?= -std=c99 -Wall -Wextra -O3 -Isrc
LDFLAGS	?= -lX11 -lXinerama

PREFIX	?= /usr/local
BIN		?= sxwm
SRC_DIR	:= src
OBJ_DIR	:= build
SRC		:= $(wildcard $(SRC_DIR)/*.c)
OBJ		:= $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN)

install: all
	@echo "Installing $(BIN) to $(DESTDIR)$(PREFIX)/bin..."
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@install -m 755 $(BIN) $(DESTDIR)$(PREFIX)/bin/$(BIN)
	@echo "Installation complete."

uninstall:
	@echo "Uninstalling $(BIN) from $(DESTDIR)$(PREFIX)/bin..."
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)
	@echo "Uninstallation complete."

clean-install: clean install

.PHONY: all clean install uninstall clean-install
