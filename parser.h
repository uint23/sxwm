#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#define MAX_ARGS 64

const char **build_argv(const char *cmd);
KeySym parse_keysym(const char *key);
int parse_mods(const char *mods, Config *user_config);
int parser(Config *user_config);

#endif /* PARSER_H */

