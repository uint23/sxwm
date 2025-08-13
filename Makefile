CONFIG ?= config.mk
include $(CONFIG)

BIN     = sxwm
SRC_DIR = src
OBJ_DIR = build

SRC = $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

MAN     = sxwm.1
