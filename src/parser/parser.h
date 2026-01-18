#ifndef SEQDIA_PARSER_H
#define SEQDIA_PARSER_H

#include "../lexer/lexer.h"
#include "../model/types.h"
#include <stdio.h>

typedef struct {
  Lexer *lexer;
  Token current;
  bool has_error;
  bool out_of_memory;
  int error_line;
  char *error_msg;
  bool has_last_message;
  size_t last_message_idx;
} Parser;

Parser *parser_new(FILE *input);
void parser_free(Parser *p);

Diagram *parser_parse(Parser *p);

#endif
