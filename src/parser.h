#pragma once
#include "defs.h"
#define MAX_ARGS 64

void handler(char *command, char *arg, unsigned int mods, KeySym keysym, Action action, Bool is_func, Config *user_config);
void parser(Config *user_config);
unsigned int parse_mods(const char *mods, Config *user_config);
KeySym parse_keysym(const char *key);
char* strip_quotes(char *s);
char* strip(char *str);
static const char** build_argv(char *cmdline);
