#include "lexer.h"
#include "../model/types.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static char *strndup_safe(const char *s, size_t n) {
  char *dup = malloc(n + 1);
  if (dup) {
    memcpy(dup, s, n);
    dup[n] = '\0';
  }
  return dup;
}

Lexer *lexer_new(FILE *input) {
  Lexer *lex = malloc(sizeof(Lexer));
  if (!lex)
    return NULL;
  lex->input = input;
  lex->line_buf = NULL;
  lex->line_buf_size = 0;
  lex->pos = 0;
  lex->line_len = 0;
  lex->line_num = 0;
  lex->col_num = 0;
  lex->eof_reached = false;
  lex->allow_multiline_quotes = false;
  lex->error_msg = NULL;
  return lex;
}

void lexer_free(Lexer *lex) {
  if (!lex)
    return;
  free(lex->line_buf);
  free(lex->error_msg);
  free(lex);
}

void token_free(Token *tok) {
  if (!tok)
    return;
  free(tok->value);
  tok->value = NULL;
}

const char *token_type_name(TokenType type) {
  switch (type) {
  case TOK_PARTICIPANT:
    return "PARTICIPANT";
  case TOK_AS:
    return "AS";
  case TOK_NOTE:
    return "NOTE";
  case TOK_LEFT:
    return "LEFT";
  case TOK_RIGHT:
    return "RIGHT";
  case TOK_OVER:
    return "OVER";
  case TOK_OF:
    return "OF";
  case TOK_END:
    return "END";
  case TOK_IDENTIFIER:
    return "IDENTIFIER";
  case TOK_QUOTED_STRING:
    return "QUOTED_STRING";
  case TOK_ARROW_SOLID:
    return "ARROW_SOLID";
  case TOK_ARROW_SOLID_LEFT:
    return "ARROW_SOLID_LEFT";
  case TOK_ARROW_OPEN:
    return "ARROW_OPEN";
  case TOK_ARROW_OPEN_LEFT:
    return "ARROW_OPEN_LEFT";
  case TOK_ARROW_DASHED:
    return "ARROW_DASHED";
  case TOK_ARROW_DASHED_LEFT:
    return "ARROW_DASHED_LEFT";
  case TOK_ARROW_X:
    return "ARROW_X";
  case TOK_ARROW_X_LEFT:
    return "ARROW_X_LEFT";
  case TOK_COLON:
    return "COLON";
  case TOK_COMMA:
    return "COMMA";
  case TOK_NEWLINE:
    return "NEWLINE";
  case TOK_EOF:
    return "EOF";
  case TOK_ERROR:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

static bool is_valid_utf8_byte(unsigned char c, int *expected_cont) {
  if (c < 0x80) {
    *expected_cont = 0;
    return true;
  } else if ((c & 0xC0) == 0x80) {
    if (*expected_cont > 0) {
      (*expected_cont)--;
      return true;
    }
    return false;
  } else if ((c & 0xE0) == 0xC0) {
    *expected_cont = 1;
    return true;
  } else if ((c & 0xF0) == 0xE0) {
    *expected_cont = 2;
    return true;
  } else if ((c & 0xF8) == 0xF0) {
    *expected_cont = 3;
    return true;
  }
  return false;
}

static bool validate_utf8(const char *s, size_t len) {
  int expected_cont = 0;
  for (size_t i = 0; i < len; i++) {
    if (!is_valid_utf8_byte((unsigned char)s[i], &expected_cont)) {
      return false;
    }
  }
  return expected_cont == 0;
}

static void normalize_line_endings(Lexer *lex) {
  size_t write = 0;
  for (size_t read = 0; read < lex->line_len; read++) {
    char c = lex->line_buf[read];
    if (c == '\r')
      continue;
    lex->line_buf[write++] = c;
  }
  lex->line_len = write;
  if (lex->line_buf)
    lex->line_buf[write] = '\0';
}

static bool read_next_line(Lexer *lex) {
  if (lex->eof_reached)
    return false;

  ssize_t len = getline(&lex->line_buf, &lex->line_buf_size, lex->input);
  if (len < 0) {
    lex->eof_reached = true;
    return false;
  }

  lex->line_len = (size_t)len;
  lex->pos = 0;
  lex->line_num++;
  lex->col_num = 1;

  normalize_line_endings(lex);

  if (!validate_utf8(lex->line_buf, lex->line_len)) {
    free(lex->error_msg);
    lex->error_msg = safe_strdup("invalid UTF-8 sequence");
    return false;
  }

  return true;
}

static char peek(Lexer *lex) {
  if (lex->pos >= lex->line_len)
    return '\0';
  return lex->line_buf[lex->pos];
}

static char peek_ahead(Lexer *lex, size_t offset) {
  if (lex->pos + offset >= lex->line_len)
    return '\0';
  return lex->line_buf[lex->pos + offset];
}

static void advance(Lexer *lex) {
  if (lex->pos < lex->line_len) {
    lex->pos++;
    lex->col_num++;
  }
}

static void skip_whitespace(Lexer *lex) {
  while (peek(lex) == ' ' || peek(lex) == '\t') {
    advance(lex);
  }
}

static bool is_identifier_char(char c) {
  return isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-';
}

static bool is_note_delimiter(char c) { return c == ':' || c == ','; }

static Token make_token(Lexer *lex, TokenType type, const char *value) {
  Token tok;
  tok.type = type;
  tok.value = value ? safe_strdup(value) : NULL;
  tok.line = lex->line_num;
  tok.col = lex->col_num;
  return tok;
}

static Token make_error(Lexer *lex, const char *msg) {
  Token tok;
  tok.type = TOK_ERROR;
  tok.value = safe_strdup(msg);
  tok.line = lex->line_num;
  tok.col = lex->col_num;
  return tok;
}

static Token lex_quoted_string(Lexer *lex, bool allow_multiline) {
  int start_line = lex->line_num;
  int start_col = lex->col_num;
  advance(lex);

  size_t cap = 64;
  size_t len = 0;
  char *buf = malloc(cap);
  if (!buf)
    return make_error(lex, "out of memory");

  while (true) {
    char c = peek(lex);
    if (c == '"') {
      advance(lex);
      buf[len] = '\0';
      Token tok = make_token(lex, TOK_QUOTED_STRING, buf);
      tok.line = start_line;
      tok.col = start_col;
      free(buf);
      return tok;
    } else if (c == '\n' || c == '\0') {
      if (!allow_multiline) {
        free(buf);
        return make_error(lex, "newline in quoted string");
      }
      if (len + 1 >= cap) {
        cap *= 2;
        char *new_buf = realloc(buf, cap);
        if (!new_buf) {
          free(buf);
          return make_error(lex, "out of memory");
        }
        buf = new_buf;
      }
      buf[len++] = '\n';
      if (!read_next_line(lex)) {
        free(buf);
        if (lex->error_msg) {
          return make_error(lex, lex->error_msg);
        }
        return make_error(lex, "unterminated quoted string");
      }
    } else {
      if (len + 1 >= cap) {
        cap *= 2;
        char *new_buf = realloc(buf, cap);
        if (!new_buf) {
          free(buf);
          return make_error(lex, "out of memory");
        }
        buf = new_buf;
      }
      buf[len++] = c;
      advance(lex);
    }
  }
}

static bool is_arrow_start(Lexer *lex) {
  char c0 = peek(lex);
  char c1 = peek_ahead(lex, 1);
  if (c0 == '-' && (c1 == '>' || c1 == '-'))
    return true;
  if (c0 == '<' && (c1 == '-' || c1 == '<'))
    return true;
  return false;
}

static Token lex_identifier(Lexer *lex) {
  size_t start = lex->pos;
  int start_col = lex->col_num;

  while (is_identifier_char(peek(lex))) {
    if (is_arrow_start(lex) || is_note_delimiter(peek(lex)))
      break;
    advance(lex);
  }

  size_t len = lex->pos - start;
  char *value = strndup_safe(lex->line_buf + start, len);
  if (!value)
    return make_error(lex, "out of memory");

  TokenType type = TOK_IDENTIFIER;
  if (strcmp(value, "participant") == 0) {
    type = TOK_PARTICIPANT;
  } else if (strcmp(value, "as") == 0) {
    type = TOK_AS;
  } else if (strcmp(value, "note") == 0) {
    type = TOK_NOTE;
  } else if (strcmp(value, "left") == 0) {
    type = TOK_LEFT;
  } else if (strcmp(value, "right") == 0) {
    type = TOK_RIGHT;
  } else if (strcmp(value, "over") == 0) {
    type = TOK_OVER;
  } else if (strcmp(value, "of") == 0) {
    type = TOK_OF;
  } else if (strcmp(value, "end") == 0) {
    type = TOK_END;
  }

  Token tok;
  tok.type = type;
  tok.value = value;
  tok.line = lex->line_num;
  tok.col = start_col;
  return tok;
}

static Token lex_arrow(Lexer *lex) {
  char c0 = peek(lex);
  char c1 = peek_ahead(lex, 1);
  char c2 = peek_ahead(lex, 2);

  int start_col = lex->col_num;
  TokenType type;
  int chars_to_advance;

  if (c0 == '-' && c1 == '>' && c2 == 'x') {
    type = TOK_ARROW_X;
    chars_to_advance = 3;
  } else if (c0 == 'x' && c1 == '<' && c2 == '-') {
    type = TOK_ARROW_X_LEFT;
    chars_to_advance = 3;
  } else if (c0 == '-' && c1 == '>' && c2 == '>') {
    type = TOK_ARROW_OPEN;
    chars_to_advance = 3;
  } else if (c0 == '<' && c1 == '<' && c2 == '-') {
    type = TOK_ARROW_OPEN_LEFT;
    chars_to_advance = 3;
  } else if (c0 == '-' && c1 == '-' && c2 == '>') {
    type = TOK_ARROW_DASHED;
    chars_to_advance = 3;
  } else if (c0 == '<' && c1 == '-' && c2 == '-') {
    type = TOK_ARROW_DASHED_LEFT;
    chars_to_advance = 3;
  } else if (c0 == '-' && c1 == '>') {
    type = TOK_ARROW_SOLID;
    chars_to_advance = 2;
  } else if (c0 == '<' && c1 == '-') {
    type = TOK_ARROW_SOLID_LEFT;
    chars_to_advance = 2;
  } else {
    return make_error(lex, "invalid arrow");
  }

  for (int i = 0; i < chars_to_advance; i++)
    advance(lex);

  Token tok;
  tok.type = type;
  tok.value = NULL;
  tok.line = lex->line_num;
  tok.col = start_col;
  return tok;
}

static bool is_whole_line_comment(Lexer *lex) {
  if (lex->pos != 0) {
    bool preceded_by_ws = false;
    for (size_t i = 0; i < lex->pos; i++) {
      char c = lex->line_buf[i];
      if (c != ' ' && c != '\t') {
        return false;
      }
      preceded_by_ws = true;
    }
    if (!preceded_by_ws)
      return false;
  }
  char c = peek(lex);
  if (c == '\'')
    return true;
  if (c == ';')
    return true;
  if (c == '/' && peek_ahead(lex, 1) == '/')
    return true;
  return false;
}

Token lexer_next(Lexer *lex) {
  if (lex->line_buf == NULL || lex->pos >= lex->line_len) {
    if (!read_next_line(lex)) {
      if (lex->error_msg) {
        return make_error(lex, lex->error_msg);
      }
      return make_token(lex, TOK_EOF, NULL);
    }
  }

  skip_whitespace(lex);

  char c = peek(lex);

  if (c == '\n' || c == '\0') {
    advance(lex);
    return make_token(lex, TOK_NEWLINE, NULL);
  }

  if (is_whole_line_comment(lex)) {
    while (peek(lex) != '\n' && peek(lex) != '\0') {
      advance(lex);
    }
    advance(lex);
    return make_token(lex, TOK_NEWLINE, NULL);
  }

  if (c == ':') {
    advance(lex);
    return make_token(lex, TOK_COLON, ":");
  }

  if (c == ',') {
    advance(lex);
    return make_token(lex, TOK_COMMA, ",");
  }

  if (c == '"') {
    return lex_quoted_string(lex, lex->allow_multiline_quotes);
  }

  if (c == '-' || c == '<' || c == 'x') {
    char c1 = peek_ahead(lex, 1);
    if ((c == '-' && (c1 == '>' || c1 == '-')) ||
        (c == '<' && (c1 == '-' || c1 == '<')) || (c == 'x' && c1 == '<')) {
      return lex_arrow(lex);
    }
  }

  if (is_identifier_char(c)) {
    return lex_identifier(lex);
  }

  char err_msg[64];
  snprintf(err_msg, sizeof(err_msg), "unexpected character '%c'", c);
  advance(lex);
  return make_error(lex, err_msg);
}

static char *read_remaining_trimmed(Lexer *lex) {
  size_t start = lex->pos;
  size_t end = lex->line_len;

  while (end > start &&
         (lex->line_buf[end - 1] == '\n' || lex->line_buf[end - 1] == '\r')) {
    end--;
  }

  while (end > start &&
         (lex->line_buf[end - 1] == ' ' || lex->line_buf[end - 1] == '\t')) {
    end--;
  }

  size_t len = end - start;
  char *msg = strndup_safe(lex->line_buf + start, len);
  if (!msg) {
    free(lex->error_msg);
    lex->error_msg = safe_strdup("out of memory");
  }

  lex->pos = lex->line_len;

  return msg;
}

char *lexer_read_message(Lexer *lex) {
  skip_whitespace(lex);

  free(lex->error_msg);
  lex->error_msg = NULL;

  if (peek(lex) == '"') {
    Token tok = lex_quoted_string(lex, true);
    if (tok.type == TOK_QUOTED_STRING) {
      char *msg = tok.value;
      return msg;
    }
    if (tok.type == TOK_ERROR && tok.value) {
      free(lex->error_msg);
      lex->error_msg = safe_strdup(tok.value);
    }
    token_free(&tok);
    if (!lex->error_msg)
      lex->error_msg = safe_strdup("out of memory");
    return NULL;
  }

  return read_remaining_trimmed(lex);
}

char *lexer_read_line(Lexer *lex) {
  free(lex->error_msg);
  lex->error_msg = NULL;

  if (peek(lex) == '"') {
    Token tok = lex_quoted_string(lex, true);
    if (tok.type == TOK_QUOTED_STRING) {
      char *line = tok.value;
      return line;
    }
    if (tok.type == TOK_ERROR && tok.value) {
      free(lex->error_msg);
      lex->error_msg = safe_strdup(tok.value);
    }
    token_free(&tok);
    if (!lex->error_msg)
      lex->error_msg = safe_strdup("out of memory");
    return NULL;
  }

  lex->pos = 0;
  return read_remaining_trimmed(lex);
}
