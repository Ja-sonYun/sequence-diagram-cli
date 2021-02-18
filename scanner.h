#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "mem.h"
#include "split.h"
#include "grammars/grammar.h"

bool is_sym(char ch);
int rdw_ignspc(Word *word, char *ch, bool *wrapped);
int rd_sym(Word *l_sym, char *line);
int rdw(Word *curw, char *ch);
#endif
