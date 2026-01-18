#ifndef SEQDIA_LEXER_H
#define SEQDIA_LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef enum {
  TOK_PARTICIPANT,
  TOK_AS,
  TOK_NOTE,
  TOK_LEFT,
  TOK_RIGHT,
  TOK_OVER,
  TOK_OF,
  TOK_END,
  TOK_IDENTIFIER,
  TOK_QUOTED_STRING,
  TOK_ARROW_SOLID,
  TOK_ARROW_SOLID_LEFT,
  TOK_ARROW_OPEN,
  TOK_ARROW_OPEN_LEFT,
  TOK_ARROW_DASHED,
  TOK_ARROW_DASHED_LEFT,
  TOK_ARROW_X,
  TOK_ARROW_X_LEFT,
  TOK_COLON,
  TOK_COMMA,
  TOK_NEWLINE,
  TOK_EOF,
  TOK_ERROR
} TokenType;

typedef struct {
  TokenType type;
  char *value;
  int line;
  int col;
} Token;

typedef struct {
  FILE *input;
  char *line_buf;
  size_t line_buf_size;
  size_t pos;
  size_t line_len;
  int line_num;
  int col_num;
  bool eof_reached;
  bool allow_multiline_quotes;
  char *error_msg;
} Lexer;

Lexer *lexer_new(FILE *input);
void lexer_free(Lexer *lex);

Token lexer_next(Lexer *lex);
void token_free(Token *tok);
const char *token_type_name(TokenType type);

char *lexer_read_message(Lexer *lex);
char *lexer_read_line(Lexer *lex);

#endif
