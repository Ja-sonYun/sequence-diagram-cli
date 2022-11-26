#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef CHECK_UPDATE
#   include "fetch.h"
#endif

int SHOW_LOG;
char *prefix;
char *suffix;
bool printRaw;

#include "renderer.h"
