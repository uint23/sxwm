# ----------------------------------------
# Paths
# ----------------------------------------
PREFIX     ?= /usr/local
SHAREDIR   ?= $(PREFIX)/share
MANPREFIX  ?= $(SHAREDIR)/man
BUILDDIR   ?= build

# ----------------------------------------
# OS and platform detection
# ----------------------------------------
UNAME_S := $(shell uname -s)

X11_INC :=
X11_LIB :=
EXTRA_LDLIBS :=

ifeq ($(UNAME_S),Linux)
    ifneq ($(shell test -d /data/data/com.termux/files/usr && echo yes),)
        X11_INC := /data/data/com.termux/files/usr/include
        X11_LIB := /data/data/com.termux/files/usr/lib

        EXTRA_LDLIBS := -landroid-wordexp
        $(info Detected Termux/Android, using $(X11_INC) $(X11_LIB))
    else
        X11_INC := /usr/include
        X11_LIB := /usr/lib
        $(info Detected Linux, using $(X11_INC) $(X11_LIB))
    endif
endif

ifeq ($(findstring BSD,$(UNAME_S)),BSD)
    ifneq ("$(wildcard /usr/X11R6)","")
        X11_INC := /usr/X11R6/include
        X11_LIB := /usr/X11R6/lib
    else ifneq ("$(wildcard /usr/local)","")
        X11_INC := /usr/local/include
        X11_LIB := /usr/local/lib
    else
        $(warning No X11 paths found on BSD, please set X11_INC/X11_LIB manually)
    endif
    $(info Detected BSD, using $(X11_INC) $(X11_LIB))
endif

# ----------------------------------------
# Compiler configuration
# ----------------------------------------
CC        ?= cc
CPPFLAGS  := -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -I$(X11_INC)
CFLAGS    := -std=c99 -pedantic -Wall -Wextra -Werror -Os
LDFLAGS   := -L$(X11_LIB)
LDLIBS    := -lX11 -lXinerama -lXcursor $(EXTRA_LDLIBS)

override CFLAGS   += $(USER_CFLAGS)
override LDFLAGS  += $(USER_LDFLAGS)
override LDLIBS   += $(USER_LDLIBS)

# ----------------------------------------
# Files
# ----------------------------------------
SRC := src/sxwm.c src/parser.c
OBJ := $(SRC:src/%.c=$(BUILDDIR)/%.o)
BIN := sxwm

# ----------------------------------------
# Build rules
# ----------------------------------------
all: $(BIN)

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/%.o: src/%.c | $(BUILDDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# ----------------------------------------
# Installation
# ----------------------------------------
install: $(BIN)
	install -Dm755 $(BIN)         $(DESTDIR)$(PREFIX)/bin/$(BIN)
	install -Dm644 docs/sxwm.1    $(DESTDIR)$(MANPREFIX)/man1/sxwm.1
	install -Dm644 default_sxwmrc $(DESTDIR)$(SHAREDIR)/sxwmrc

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN) \
	      $(DESTDIR)$(MANPREFIX)/man1/sxwm.1 \
	      $(DESTDIR)$(SHAREDIR)/sxwmrc

# ----------------------------------------
# Cleanup
# ----------------------------------------
clean:
	rm -rf $(BUILDDIR) $(BIN)

distclean: clean
	rm -f compile_flags.txt

# ----------------------------------------
# Clangd / editor support
# ----------------------------------------
clangd:
	printf "%s\n" $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) > compile_flags.txt

.PHONY: all clean distclean install uninstall clangd