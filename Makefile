# tools
CC ?= cc
PKG_CONFIG ?= pkg-config

# install dirs
PREFIX ?= /usr/local
DESTDIR ?=
BIN := sxwm
MAN := docs/sxwm.1
MAN_DIR := $(PREFIX)/share/man/man1
XSESSIONS := $(DESTDIR)$(PREFIX)/share/xsessions

# layout
SRC_DIR := src/
OBJ_DIR := build/
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

# flags
CPPFLAGS ?= -Isrc -D_FORTIFY_SOURCE=2

# compile flags + warnings, hardening
CFLAGS ?= -std=c99 -Os -pipe \
          -Wall -Wextra -Wformat=2 -Werror=format-security \
          -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings \
          -Wmissing-prototypes -Wstrict-prototypes -Wswitch-enum \
          -Wundef -Wvla -fno-common -fno-strict-aliasing \
          -fstack-protector-strong -fPIE

# linker
LDFLAGS ?= -Wl,-O1 -pie

# libraries
LDLIBS ?= -lX11 -lXinerama -lXcursor

# prefer pkg-confgi
ifneq ($(shell $(PKG_CONFIG) --exists x11 xinerama xcursor && echo yes),)
CPPFLAGS += $(shell $(PKG_CONFIG) --cflags x11 xinerama xcursor)
LDLIBS := $(shell $(PKG_CONFIG) --libs   x11 xinerama xcursor)
endif

.PHONY: all clean install uninstall clangd
.SUFFIXES:

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(DEP)

$(OBJ_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(OBJ_DIR) $(BIN)

install: all
	@echo "installing $(BIN) to $(DESTDIR)$(PREFIX)/bin..."
	@mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	@install -m 755 $(BIN) "$(DESTDIR)$(PREFIX)/bin/$(BIN)"
	@echo "installing sxwm.desktop to $(XSESSIONS)..."
	@mkdir -p "$(XSESSIONS)"
	@install -m 644 docs/sxwm.desktop "$(XSESSIONS)/sxwm.desktop"
	@echo "installing man page to $(DESTDIR)$(MAN_DIR)..."
	@mkdir -p "$(DESTDIR)$(MAN_DIR)"
	@install -m 644 $(MAN) "$(DESTDIR)$(MAN_DIR)/"
	@echo "copying default config to $(DESTDIR)$(PREFIX)/share/sxwmrc..."
	@mkdir -p "$(DESTDIR)$(PREFIX)/share"
	@install -m 644 default_sxwmrc "$(DESTDIR)$(PREFIX)/share/sxwmrc"
	@echo "installation complete :)"

uninstall:
	@echo "uninstalling $(BIN) from $(DESTDIR)$(PREFIX)/bin..."
	@rm -f "$(DESTDIR)$(PREFIX)/bin/$(BIN)"
	@echo "uninstalling sxwm.desktop from $(XSESSIONS)..."
	@rm -f "$(XSESSIONS)/sxwm.desktop"
	@echo "uninstalling man page from $(DESTDIR)$(MAN_DIR)..."
	@rm -f "$(DESTDIR)$(MAN_DIR)/$(MAN)"
	@echo "uninstallation complete :)"

# dev tools
clangd:
	@echo "generating compile_flags.txt"
	@rm -f compile_flags.txt
	@for flag in $(CPPFLAGS) $(CFLAGS); do echo $$flag >> compile_flags.txt; done
