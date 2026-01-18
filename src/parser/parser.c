#include "parser.h"
#include "../lexer/lexer.h"
#include "../model/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser *parser_new(FILE *input) {
  Parser *p = malloc(sizeof(Parser));
  if (!p)
    return NULL;
  p->lexer = lexer_new(input);
  if (!p->lexer) {
    free(p);
    return NULL;
  }
  p->current.type = TOK_EOF;
  p->current.value = NULL;
  p->has_error = false;
  p->out_of_memory = false;
  p->error_line = 0;
  p->error_msg = NULL;
  p->has_last_message = false;
  p->last_message_idx = 0;
  return p;
}

void parser_free(Parser *p) {
  if (!p)
    return;
  token_free(&p->current);
  lexer_free(p->lexer);
  free(p->error_msg);
  free(p);
}

static void advance(Parser *p) {
  token_free(&p->current);
  p->current = lexer_next(p->lexer);
}

static void set_error(Parser *p, int line, const char *msg) {
  if (p->has_error)
    return;
  p->has_error = true;
  p->error_line = line;
  free(p->error_msg);
  p->error_msg = safe_strdup(msg);
  if (!p->error_msg) {
    p->out_of_memory = true;
  }
}

static void set_error_oom(Parser *p, int line) {
  if (p->has_error)
    return;
  p->has_error = true;
  p->out_of_memory = true;
  p->error_line = line;
  free(p->error_msg);
  p->error_msg = NULL;
}

static bool expect(Parser *p, TokenType type) {
  if (p->current.type != type) {
    char msg[128];
    snprintf(msg, sizeof(msg), "expected %s, got %s", token_type_name(type),
             token_type_name(p->current.type));
    set_error(p, p->current.line, msg);
    return false;
  }
  return true;
}

static void skip_to_newline(Parser *p) {
  while (p->current.type != TOK_NEWLINE && p->current.type != TOK_EOF) {
    advance(p);
  }
}

static ArrowType token_to_arrow_type(TokenType type) {
  switch (type) {
  case TOK_ARROW_SOLID:
    return ARROW_SOLID;
  case TOK_ARROW_SOLID_LEFT:
    return ARROW_SOLID_LEFT;
  case TOK_ARROW_OPEN:
    return ARROW_OPEN;
  case TOK_ARROW_OPEN_LEFT:
    return ARROW_OPEN_LEFT;
  case TOK_ARROW_DASHED:
    return ARROW_DASHED;
  case TOK_ARROW_DASHED_LEFT:
    return ARROW_DASHED_LEFT;
  case TOK_ARROW_X:
    return ARROW_X;
  case TOK_ARROW_X_LEFT:
    return ARROW_X_LEFT;
  default:
    return ARROW_SOLID;
  }
}

static bool is_arrow_token(TokenType type) {
  return type == TOK_ARROW_SOLID || type == TOK_ARROW_SOLID_LEFT ||
         type == TOK_ARROW_OPEN || type == TOK_ARROW_OPEN_LEFT ||
         type == TOK_ARROW_DASHED || type == TOK_ARROW_DASHED_LEFT ||
         type == TOK_ARROW_X || type == TOK_ARROW_X_LEFT;
}

static char *parse_actor_name(Parser *p) {
  if (p->current.type == TOK_IDENTIFIER ||
      p->current.type == TOK_QUOTED_STRING) {
    char *name = safe_strdup(p->current.value);
    if (!name) {
      set_error_oom(p, p->current.line);
      return NULL;
    }
    advance(p);
    return name;
  }

  set_error(p, p->current.line, "expected actor name");
  return NULL;
}

static size_t ensure_participant(Parser *p, Diagram *d, const char *name) {
  size_t idx = diagram_find_participant(d, name);
  if (idx != (size_t)-1) {
    return idx;
  }
  idx = diagram_add_participant(d, name, NULL);
  if (idx == (size_t)-1) {
    set_error_oom(p, p->current.line);
  }
  return idx;
}

static bool parse_participant(Parser *p, Diagram *d) {
  p->lexer->allow_multiline_quotes = true;
  advance(p);

  if (p->current.type == TOK_ERROR) {
    p->lexer->allow_multiline_quotes = false;
    set_error(p, p->current.line, p->current.value);
    return false;
  }

  char *name = NULL;
  char *alias = NULL;
  if (p->current.type == TOK_IDENTIFIER) {
    name = safe_strdup(p->current.value);
    if (!name) {
      p->lexer->allow_multiline_quotes = false;
      set_error_oom(p, p->current.line);
      return false;
    }
    advance(p);
  } else if (p->current.type == TOK_QUOTED_STRING) {
    name = safe_strdup(p->current.value);
    if (!name) {
      p->lexer->allow_multiline_quotes = false;
      set_error_oom(p, p->current.line);
      return false;
    }
    advance(p);
  } else {
    p->lexer->allow_multiline_quotes = false;
    set_error(p, p->current.line, "expected participant name");
    return false;
  }

  if (p->current.type == TOK_AS) {
    advance(p);
    if (p->current.type == TOK_ERROR) {
      free(name);
      p->lexer->allow_multiline_quotes = false;
      set_error(p, p->current.line, p->current.value);
      return false;
    }
    if (!expect(p, TOK_IDENTIFIER)) {
      free(name);
      p->lexer->allow_multiline_quotes = false;
      return false;
    }
    alias = safe_strdup(p->current.value);
    if (!alias) {
      free(name);
      p->lexer->allow_multiline_quotes = false;
      set_error_oom(p, p->current.line);
      return false;
    }
    advance(p);
  }

  p->lexer->allow_multiline_quotes = false;

  size_t idx = diagram_add_participant(d, name, alias);
  if (idx == (size_t)-1) {
    free(name);
    free(alias);
    set_error(p, p->current.line, "participant name or alias already used");
    return false;
  }

  free(name);
  free(alias);

  skip_to_newline(p);
  return true;
}

static bool parse_message(Parser *p, Diagram *d) {
  char *from_name = parse_actor_name(p);
  if (!from_name)
    return false;

  if (!is_arrow_token(p->current.type)) {
    set_error(p, p->current.line, "expected arrow");
    free(from_name);
    return false;
  }
  ArrowType arrow = token_to_arrow_type(p->current.type);
  advance(p);

  char *to_name = parse_actor_name(p);
  if (!to_name) {
    free(from_name);
    return false;
  }

  if (!expect(p, TOK_COLON)) {
    free(from_name);
    free(to_name);
    return false;
  }

  int message_line = p->current.line;

  token_free(&p->current);
  p->current.type = TOK_EOF;
  p->current.value = NULL;

  char *msg_text = lexer_read_message(p->lexer);
  if (!msg_text) {
    free(from_name);
    free(to_name);
    if (p->lexer->error_msg) {
      if (strcmp(p->lexer->error_msg, "out of memory") == 0) {
        set_error_oom(p, message_line);
      } else {
        set_error(p, message_line, p->lexer->error_msg);
      }
    } else {
      set_error_oom(p, message_line);
    }
    return false;
  }

  advance(p);

  bool is_left_arrow = (arrow == ARROW_SOLID_LEFT || arrow == ARROW_OPEN_LEFT ||
                        arrow == ARROW_DASHED_LEFT || arrow == ARROW_X_LEFT);

  size_t left_idx = ensure_participant(p, d, from_name);
  if (left_idx == (size_t)-1) {
    free(from_name);
    free(to_name);
    free(msg_text);
    return false;
  }

  size_t right_idx = ensure_participant(p, d, to_name);
  if (right_idx == (size_t)-1) {
    free(from_name);
    free(to_name);
    free(msg_text);
    return false;
  }

  size_t from_idx, to_idx;
  ArrowType final_arrow = arrow;
  if (is_left_arrow) {
    from_idx = right_idx;
    to_idx = left_idx;
    if (arrow == ARROW_SOLID_LEFT)
      final_arrow = ARROW_SOLID;
    else if (arrow == ARROW_OPEN_LEFT)
      final_arrow = ARROW_OPEN;
    else if (arrow == ARROW_DASHED_LEFT)
      final_arrow = ARROW_DASHED;
    else if (arrow == ARROW_X_LEFT)
      final_arrow = ARROW_X;
  } else {
    from_idx = left_idx;
    to_idx = right_idx;
  }

  if (!diagram_add_message(d, from_idx, to_idx, final_arrow, msg_text)) {
    free(from_name);
    free(to_name);
    free(msg_text);
    set_error_oom(p, p->current.line);
    return false;
  }

  p->has_last_message = true;
  p->last_message_idx = d->message_count - 1;

  free(from_name);
  free(to_name);
  free(msg_text);

  return true;
}

static bool parse_note(Parser *p, Diagram *d) {
  advance(p);

  NotePosition position;
  if (p->current.type == TOK_LEFT) {
    position = NOTE_LEFT;
  } else if (p->current.type == TOK_RIGHT) {
    position = NOTE_RIGHT;
  } else if (p->current.type == TOK_OVER) {
    position = NOTE_OVER;
  } else {
    set_error(p, p->current.line, "expected note position");
    return false;
  }
  advance(p);

  bool attach_to_message = false;

  if (position == NOTE_LEFT || position == NOTE_RIGHT) {
    if (p->current.type == TOK_OF) {
      advance(p);
    } else if (!p->has_last_message) {
      set_error(p, p->current.line,
                "note left/right requires a preceding message");
      return false;
    } else {
      attach_to_message = true;
    }
  }

  char *from_name = NULL;
  if (position == NOTE_LEFT || position == NOTE_RIGHT) {
    if (!attach_to_message) {
      from_name = parse_actor_name(p);
      if (!from_name)
        return false;
    }
  } else {
    from_name = parse_actor_name(p);
    if (!from_name)
      return false;
  }

  char *to_name = NULL;
  if (position == NOTE_OVER && p->current.type == TOK_COMMA) {
    advance(p);
    to_name = parse_actor_name(p);
    if (!to_name) {
      free(from_name);
      return false;
    }
  }

  if (position == NOTE_OVER && !to_name && p->current.type != TOK_COLON &&
      p->current.type != TOK_NEWLINE && p->current.type != TOK_EOF) {
    to_name = parse_actor_name(p);
    if (!to_name) {
      free(from_name);
      return false;
    }
  }

  bool inline_note = false;
  if (p->current.type == TOK_COLON) {
    inline_note = true;
  } else if (p->current.type != TOK_NEWLINE && p->current.type != TOK_EOF) {
    set_error(p, p->current.line, "expected ':' or newline");
    free(from_name);
    free(to_name);
    return false;
  }

  int note_line = p->current.line;
  char *note_text = NULL;

  if (inline_note) {
    token_free(&p->current);
    p->current.type = TOK_EOF;
    p->current.value = NULL;
    note_text = lexer_read_message(p->lexer);
    if (!note_text) {
      if (p->lexer->error_msg) {
        if (strcmp(p->lexer->error_msg, "out of memory") == 0) {
          set_error_oom(p, note_line);
        } else {
          set_error(p, note_line, p->lexer->error_msg);
        }
      } else {
        set_error_oom(p, note_line);
      }
      free(from_name);
      free(to_name);
      return false;
    }
    advance(p);
  } else {
    advance(p);

    size_t cap = 128;
    size_t len = 0;
    note_text = malloc(cap);
    if (!note_text) {
      set_error_oom(p, note_line);
      free(from_name);
      free(to_name);
      return false;
    }

    bool end_note_found = false;
    while (p->current.type != TOK_EOF) {
      if (p->current.type == TOK_END) {
        int end_line = p->current.line;
        advance(p);
        if (!expect(p, TOK_NOTE)) {
          free(note_text);
          free(from_name);
          free(to_name);
          return false;
        }
        advance(p);
        if (p->current.type != TOK_NEWLINE && p->current.type != TOK_EOF) {
          set_error(p, end_line, "expected end note on its own line");
          free(note_text);
          free(from_name);
          free(to_name);
          return false;
        }
        if (p->current.type == TOK_NEWLINE)
          advance(p);
        end_note_found = true;
        break;
      }

      if (p->current.type == TOK_NEWLINE) {
        if (len + 1 >= cap) {
          cap *= 2;
          char *new_buf = realloc(note_text, cap);
          if (!new_buf) {
            free(note_text);
            set_error_oom(p, p->current.line);
            free(from_name);
            free(to_name);
            return false;
          }
          note_text = new_buf;
        }
        note_text[len++] = '\n';
        advance(p);
        continue;
      }

      token_free(&p->current);
      p->current.type = TOK_EOF;
      p->current.value = NULL;

      char *line = lexer_read_line(p->lexer);
      if (!line) {
        if (p->lexer->error_msg) {
          if (strcmp(p->lexer->error_msg, "out of memory") == 0) {
            set_error_oom(p, p->current.line);
          } else {
            set_error(p, p->current.line, p->lexer->error_msg);
          }
        } else {
          set_error_oom(p, p->current.line);
        }
        free(note_text);
        free(from_name);
        free(to_name);
        return false;
      }

      size_t line_len = strlen(line);
      if (len + line_len + 1 >= cap) {
        while (len + line_len + 1 >= cap)
          cap *= 2;
        char *new_buf = realloc(note_text, cap);
        if (!new_buf) {
          free(line);
          free(note_text);
          set_error_oom(p, p->current.line);
          free(from_name);
          free(to_name);
          return false;
        }
        note_text = new_buf;
      }
      memcpy(note_text + len, line, line_len);
      len += line_len;
      free(line);

      advance(p);
      if (p->current.type != TOK_END && p->current.type != TOK_EOF &&
          p->current.type != TOK_NEWLINE) {
        if (len + 1 >= cap) {
          cap *= 2;
          char *new_buf = realloc(note_text, cap);
          if (!new_buf) {
            free(note_text);
            set_error_oom(p, p->current.line);
            free(from_name);
            free(to_name);
            return false;
          }
          note_text = new_buf;
        }
        note_text[len++] = '\n';
      }
    }

    if (!end_note_found) {
      set_error(p, note_line, "unterminated note");
      free(note_text);
      free(from_name);
      free(to_name);
      return false;
    }

    if (p->current.type == TOK_EOF && !p->has_error) {
      if (len > 0 && note_text[len - 1] == '\n')
        note_text[len - 1] = '\0';
      else
        note_text[len] = '\0';
    } else {
      note_text[len] = '\0';
    }
  }

  size_t from_idx = 0;
  if (!attach_to_message) {
    from_idx = ensure_participant(p, d, from_name);
    if (from_idx == (size_t)-1) {
      free(note_text);
      free(from_name);
      free(to_name);
      return false;
    }
  } else {
    Message *m = &d->messages[p->last_message_idx];
    size_t left_idx = m->from_idx < m->to_idx ? m->from_idx : m->to_idx;
    size_t right_idx = m->from_idx < m->to_idx ? m->to_idx : m->from_idx;
    from_idx = (position == NOTE_LEFT) ? left_idx : right_idx;
  }

  size_t to_idx = from_idx;
  if (position == NOTE_OVER && to_name) {
    to_idx = ensure_participant(p, d, to_name);
    if (to_idx == (size_t)-1) {
      free(note_text);
      free(from_name);
      free(to_name);
      return false;
    }
  }

  if (position == NOTE_OVER && to_name && to_idx < from_idx) {
    size_t tmp = from_idx;
    from_idx = to_idx;
    to_idx = tmp;
  }

  size_t note_index = d->note_count;
  if (!diagram_add_note(d, position, from_idx, to_idx, note_text)) {
    set_error_oom(p, note_line);
    free(note_text);
    free(from_name);
    free(to_name);
    return false;
  }

  if (attach_to_message) {
    Message *m = &d->messages[p->last_message_idx];
    if (position == NOTE_LEFT) {
      if (m->inline_left_note_idx != (size_t)-1) {
        set_error(p, note_line, "duplicate inline left note");
        note_free(&d->notes[note_index]);
        d->note_count--;
        d->event_count--;
        free(note_text);
        free(from_name);
        free(to_name);
        return false;
      }
      m->inline_left_note_idx = note_index;
    } else if (position == NOTE_RIGHT) {
      if (m->inline_right_note_idx != (size_t)-1) {
        set_error(p, note_line, "duplicate inline right note");
        note_free(&d->notes[note_index]);
        d->note_count--;
        d->event_count--;
        free(note_text);
        free(from_name);
        free(to_name);
        return false;
      }
      m->inline_right_note_idx = note_index;
    }
    d->event_count--;
  }

  free(note_text);
  free(from_name);
  free(to_name);
  return true;
}

Diagram *parser_parse(Parser *p) {
  Diagram *d = diagram_new();
  if (!d) {
    set_error_oom(p, 0);
    return NULL;
  }

  advance(p);

  while (p->current.type != TOK_EOF) {
    if (p->current.type == TOK_ERROR) {
      if (p->current.value && strcmp(p->current.value, "out of memory") == 0) {
        set_error_oom(p, p->current.line);
      } else {
        set_error(p, p->current.line, p->current.value);
      }
      diagram_free(d);
      return NULL;
    }

    if (p->current.type == TOK_NEWLINE) {
      advance(p);
      continue;
    }

    if (p->current.type == TOK_PARTICIPANT) {
      if (!parse_participant(p, d)) {
        p->lexer->allow_multiline_quotes = false;
        diagram_free(d);
        return NULL;
      }
      continue;
    }

    if (p->current.type == TOK_NOTE) {
      if (!parse_note(p, d)) {
        p->lexer->allow_multiline_quotes = false;
        diagram_free(d);
        return NULL;
      }
      continue;
    }

    if (p->current.type == TOK_IDENTIFIER ||
        p->current.type == TOK_QUOTED_STRING) {
      if (!parse_message(p, d)) {
        p->lexer->allow_multiline_quotes = false;
        diagram_free(d);
        return NULL;
      }
      continue;
    }

    set_error(p, p->current.line, "unexpected token");
    diagram_free(d);
    return NULL;
  }

  if (p->has_error) {
    diagram_free(d);
    return NULL;
  }

  return d;
}
