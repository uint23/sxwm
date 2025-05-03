#pragma once
#include "defs.h"
#define MAX_ARGS 64

const char **build_argv(const char *cmd);
int parser(Config *user_config);
int parse_mods(const char *mods, Config *user_config);
KeySym parse_keysym(const char *key);