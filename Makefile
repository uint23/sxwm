include config.mk

SRC = $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(OBJ_DIR) $(BIN)

install: all
	@echo "Installing $(BIN) to $(DESTDIR)$(PREFIX)/bin..."
	@mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	@install -m 755 $(BIN) "$(DESTDIR)$(PREFIX)/bin/$(BIN)"
	@echo "Installing sxwm.desktop to $(XSESSIONS)..."
	@mkdir -p "$(XSESSIONS)"
	@install -m 644 sxwm.desktop "$(XSESSIONS)/sxwm.desktop"
	@echo "Installing man page to $(DESTDIR)$(MAN_DIR)..."
	@mkdir -p $(DESTDIR)$(MAN_DIR)
	@install -m 644 $(MAN) $(DESTDIR)$(MAN_DIR)/
	@echo "Copying default configuration to $(DESTDIR)$(PREFIX)/share/sxwmrc..."
	@mkdir -p "$(DESTDIR)$(PREFIX)/share"
	@install -m 644 default_sxwmrc "$(DESTDIR)$(PREFIX)/share/sxwmrc"
	@echo "Installation complete."

uninstall:
	@echo "Uninstalling $(BIN) from $(DESTDIR)$(PREFIX)/bin..."
	@rm -f "$(DESTDIR)$(PREFIX)/bin/$(BIN)"
	@echo "Uninstalling sxwm.desktop from $(XSESSIONS)..."
	@rm -f "$(XSESSIONS)/sxwm.desktop"
	@echo "Uninstalling man page from $(DESTDIR)$(MAN_DIR)..."
	@rm -f $(DESTDIR)$(MAN_DIR)/$(MAN)
	@echo "Uninstallation complete."

.PHONY: all clean install uninstall
