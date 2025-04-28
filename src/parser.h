#pragma once
#include "defs.h"
#define MAX_ARGS 64

void handler(char *command, char *arg, int mods, KeySym keysym, Action action, Bool is_func, Config *user_config);
int parser(Config *user_config);
unsigned int parse_mods(const char *mods, Config *user_config);
KeySym parse_keysym(const char *key);
