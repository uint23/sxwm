CC      ?= gcc
CFLAGS  ?= -std=c99 -Wall -Wextra -O3 -Isrc
LDFLAGS ?= -lX11 -lXinerama -lXcursor

PREFIX  ?= /usr/local
BIN     := sxwm
SRC_DIR := src
OBJ_DIR := build
SRC     := $(wildcard $(SRC_DIR)/*.c)
OBJ     := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

XSESSIONS := $(DESTDIR)$(PREFIX)/share/xsessions

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

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
	@echo "Installing sxwm.desktop to $(XSESSIONS)..."
	@mkdir -p $(XSESSIONS)
	@install -m 644 sxwm.desktop $(XSESSIONS)/sxwm.desktop
	@echo "Installation complete."

uninstall:
	@echo "Uninstalling $(BIN) from $(DESTDIR)$(PREFIX)/bin..."
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)
	@echo "Uninstalling sxwm.desktop from $(XSESSIONS)..."
	@rm -f $(XSESSIONS)/sxwm.desktop
	@echo "Uninstallation complete."

.PHONY: all clean install uninstall
