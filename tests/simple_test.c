#include "../src/lexer/lexer.h"
#include "../src/model/types.h"
#include "../src/parser/parser.h"
#include "../src/renderer/renderer.h"
#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static FILE *open_memory_stream(const char *input) {
  FILE *tmp = tmpfile();
  if (!tmp) {
    return NULL;
  }
  size_t len = strlen(input);
  if (len > 0) {
    if (fwrite(input, 1, len, tmp) != len) {
      fclose(tmp);
      return NULL;
    }
  }
  rewind(tmp);
  return tmp;
}

static char *read_stream(FILE *stream) {
  if (fseek(stream, 0, SEEK_END) != 0) {
    return NULL;
  }
  long size = ftell(stream);
  if (size < 0) {
    return NULL;
  }
  rewind(stream);

  char *buffer = malloc((size_t)size + 1);
  if (!buffer) {
    return NULL;
  }
  size_t read_len = fread(buffer, 1, (size_t)size, stream);
  buffer[read_len] = '\0';
  return buffer;
}

static char *read_file(const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    return NULL;
  }
  char *content = read_stream(file);
  fclose(file);
  return content;
}

static char *render_fixture(const char *input_path, RenderMode mode) {
  FILE *input = fopen(input_path, "r");
  assert(input != NULL);

  Parser *parser = parser_new(input);
  assert(parser != NULL);

  Diagram *diagram = parser_parse(parser);
  assert(diagram != NULL);

  FILE *tmp = tmpfile();
  assert(tmp != NULL);

  bool ok = render_diagram(diagram, mode, tmp);
  fflush(tmp);

  char *output = read_stream(tmp);
  fclose(tmp);

  diagram_free(diagram);
  parser_free(parser);
  fclose(input);

  assert(ok);
  assert(output != NULL);
  return output;
}

static void test_utf8_display_width(void) {
  assert(utf8_display_width("hello") == 5);
  assert(utf8_display_width("") == 0);
  assert(utf8_display_width(NULL) == 0);
  printf("[PASS] utf8_display_width\n");
}

static void test_participant_new_free(void) {
  Participant *p = participant_new("Alice", NULL);
  assert(p != NULL);
  assert(strcmp(p->name, "Alice") == 0);
  assert(p->alias == NULL);
  assert(p->display_width == 5);
  participant_free(p);
  free(p);

  Participant *p2 = participant_new("Bob Builder", "Bob");
  assert(p2 != NULL);
  assert(strcmp(p2->name, "Bob Builder") == 0);
  assert(strcmp(p2->alias, "Bob") == 0);
  assert(p2->display_width == 11); // display_width based on name, not alias
  participant_free(p2);
  free(p2);
  printf("[PASS] participant_new_free\n");
}

static void test_message_new_free(void) {
  Message *m = message_new(0, 1, ARROW_SOLID, "hello");
  assert(m != NULL);
  assert(m->from_idx == 0);
  assert(m->to_idx == 1);
  assert(m->arrow == ARROW_SOLID);
  assert(strcmp(m->text, "hello") == 0);
  assert(m->is_self == false);
  message_free(m);
  free(m);

  Message *m2 = message_new(0, 0, ARROW_DASHED, "self");
  assert(m2 != NULL);
  assert(m2->is_self == true);
  message_free(m2);
  free(m2);

  Note *note = note_new(NOTE_RIGHT, 0, 0, "note");
  assert(note != NULL);
  assert(note->position == NOTE_RIGHT);
  assert(note->from_idx == 0);
  assert(note->to_idx == 0);
  assert(strcmp(note->text, "note") == 0);
  assert(note->line_count == 1);
  note_free(note);
  free(note);

  printf("[PASS] message_new_free\n");
}

static void test_diagram_lifecycle(void) {
  Diagram *d = diagram_new();
  assert(d != NULL);
  assert(d->participant_count == 0);
  assert(d->message_count == 0);

  size_t alice = diagram_add_participant(d, "Alice", NULL);
  assert(alice == 0);
  assert(d->participant_count == 1);

  size_t bob = diagram_add_participant(d, "Bob Builder", "Bob");
  assert(bob == 1);
  assert(d->participant_count == 2);

  size_t found = diagram_find_participant(d, "Alice");
  assert(found == 0);

  found = diagram_find_participant(d, "Bob");
  assert(found == 1);

  found = diagram_find_participant(d, "Bob Builder");
  assert(found == 1);

  found = diagram_find_participant(d, "Carol");
  assert(found == (size_t)-1);

  size_t alice_dup = diagram_add_participant(d, "Alice", NULL);
  assert(alice_dup == (size_t)-1);
  assert(d->participant_count == 2);

  assert(diagram_add_message(d, 0, 1, ARROW_SOLID, "hello"));
  assert(d->message_count == 1);
  assert(d->event_count == 1);

  assert(diagram_add_message(d, 0, 0, ARROW_DASHED, "self"));
  assert(d->message_count == 2);
  assert(d->event_count == 2);

  assert(diagram_add_note(d, NOTE_LEFT, 0, 0, "note"));
  assert(d->note_count == 1);
  assert(d->event_count == 3);

  diagram_free(d);
  printf("[PASS] diagram_lifecycle\n");
}

static void test_lexer_arrows(void) {
  const char *input = "-> <-\n->> <<-\n--> <--\n->x x<-\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);

  Lexer *lex = lexer_new(f);
  assert(lex != NULL);

  Token tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_SOLID);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_SOLID_LEFT);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_NEWLINE);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_OPEN);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_OPEN_LEFT);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_NEWLINE);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_DASHED);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_DASHED_LEFT);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_NEWLINE);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_X);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_X_LEFT);
  token_free(&tok);

  lexer_free(lex);
  fclose(f);
  printf("[PASS] lexer_arrows\n");
}

static void test_lexer_participant(void) {
  const char *input = "participant Alice\nparticipant \"Bob B\" as Bob\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Lexer *lex = lexer_new(f);

  Token tok = lexer_next(lex);
  assert(tok.type == TOK_PARTICIPANT);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_IDENTIFIER);
  assert(strcmp(tok.value, "Alice") == 0);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_NEWLINE);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_PARTICIPANT);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_QUOTED_STRING);
  assert(strcmp(tok.value, "Bob B") == 0);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_AS);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_IDENTIFIER);
  assert(strcmp(tok.value, "Bob") == 0);
  token_free(&tok);

  lexer_free(lex);
  fclose(f);
  printf("[PASS] lexer_participant\n");
}

static void test_lexer_comments(void) {
  const char *input =
      "' comment line\n// another comment\n; semicolon comment\nAlice\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Lexer *lex = lexer_new(f);

  Token tok = lexer_next(lex);
  assert(tok.type == TOK_NEWLINE);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_NEWLINE);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_NEWLINE);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_IDENTIFIER);
  assert(strcmp(tok.value, "Alice") == 0);
  token_free(&tok);

  lexer_free(lex);
  fclose(f);
  printf("[PASS] lexer_comments\n");
}

static void test_lexer_message(void) {
  const char *input = "Alice->Bob: hello\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Lexer *lex = lexer_new(f);

  Token tok = lexer_next(lex);
  assert(tok.type == TOK_IDENTIFIER);
  assert(strcmp(tok.value, "Alice") == 0);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ARROW_SOLID);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_IDENTIFIER);
  assert(strcmp(tok.value, "Bob") == 0);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_COLON);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_IDENTIFIER);
  assert(strcmp(tok.value, "hello") == 0);
  token_free(&tok);

  lexer_free(lex);
  fclose(f);
  printf("[PASS] lexer_message\n");
}

static void test_lexer_multiline_participant_error(void) {
  const char *input = "participant \"A\nB\"\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Lexer *lex = lexer_new(f);

  Token tok = lexer_next(lex);
  assert(tok.type == TOK_PARTICIPANT);
  token_free(&tok);

  tok = lexer_next(lex);
  assert(tok.type == TOK_ERROR);
  assert(strcmp(tok.value, "newline in quoted string") == 0);
  token_free(&tok);

  lexer_free(lex);
  fclose(f);
  printf("[PASS] lexer_multiline_participant_error\n");
}

static void test_lexer_multiline_participant(void) {
  const char *input = "participant \"A\nB\" as Alice\nparticipant \"Bob B\" "
                      "as Bob\nAlice->Bob: hi\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Parser *p = parser_new(f);
  assert(p != NULL);

  Diagram *d = parser_parse(p);
  assert(d != NULL);

  assert(d->participant_count == 2);
  assert(d->message_count == 1);
  assert(d->event_count == 1);

  assert(strcmp(d->participants[0].name, "A\nB") == 0);
  assert(strcmp(d->participants[0].alias, "Alice") == 0);

  assert(strcmp(d->participants[1].name, "Bob B") == 0);
  assert(strcmp(d->participants[1].alias, "Bob") == 0);

  assert(d->messages[0].from_idx == 0);
  assert(d->messages[0].to_idx == 1);
  assert(d->messages[0].arrow == ARROW_SOLID);
  assert(strcmp(d->messages[0].text, "hi") == 0);

  diagram_free(d);
  parser_free(p);
  fclose(f);
  printf("[PASS] parser_basic\n");
}

static void test_parser_implicit_participants(void) {
  const char *input = "Alice->Bob: hello\nBob->Carol: world\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Parser *p = parser_new(f);

  Diagram *d = parser_parse(p);
  assert(d != NULL);
  assert(d->participant_count == 3);
  assert(d->message_count == 2);
  assert(d->event_count == 2);

  assert(strcmp(d->participants[0].name, "Alice") == 0);
  assert(strcmp(d->participants[1].name, "Bob") == 0);
  assert(strcmp(d->participants[2].name, "Carol") == 0);

  diagram_free(d);
  parser_free(p);
  fclose(f);
  printf("[PASS] parser_implicit_participants\n");
}

static void test_parser_participant_conflict(void) {
  const char *input =
      "participant Alice\nparticipant Alice\nparticipant Bob as Alice\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Parser *p = parser_new(f);

  Diagram *d = parser_parse(p);
  assert(d == NULL);
  assert(p->has_error);
  assert(p->error_msg != NULL);
  assert(strcmp(p->error_msg, "participant name or alias already used") == 0);

  parser_free(p);
  fclose(f);
  printf("[PASS] parser_participant_conflict\n");
}

static void test_parser_all_arrows(void) {
  const char *input = "A->B: solid\nB<-A: solid-left\nA->>B: open\nB<<-A: "
                      "open-left\nA-->B: dashed\nB<--A: dashed-left\n"
                      "A->xB: x\nB x<-A: x-left\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Parser *p = parser_new(f);

  Diagram *d = parser_parse(p);
  assert(d != NULL);
  assert(d->message_count == 8);
  assert(d->event_count == 8);

  assert(d->messages[0].arrow == ARROW_SOLID);
  assert(d->messages[1].arrow == ARROW_SOLID);
  assert(d->messages[2].arrow == ARROW_OPEN);
  assert(d->messages[3].arrow == ARROW_OPEN);
  assert(d->messages[4].arrow == ARROW_DASHED);
  assert(d->messages[5].arrow == ARROW_DASHED);
  assert(d->messages[6].arrow == ARROW_X);
  assert(d->messages[7].arrow == ARROW_X);

  diagram_free(d);
  parser_free(p);
  fclose(f);
  printf("[PASS] parser_all_arrows\n");
}

static void test_parser_self_message(void) {
  const char *input = "Alice->Alice: self\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Parser *p = parser_new(f);

  Diagram *d = parser_parse(p);
  assert(d != NULL);
  assert(d->message_count == 1);
  assert(d->event_count == 1);
  assert(d->messages[0].is_self == true);

  diagram_free(d);
  parser_free(p);
  fclose(f);
  printf("[PASS] parser_self_message\n");
}

static void test_parser_comments(void) {
  const char *input = "' comment\nparticipant Alice\n// another "
                      "comment\nAlice->Bob: hi\n; third comment\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Parser *p = parser_new(f);

  Diagram *d = parser_parse(p);
  assert(d != NULL);
  assert(d->participant_count == 2);
  assert(d->message_count == 1);
  assert(d->event_count == 1);

  diagram_free(d);
  parser_free(p);
  fclose(f);
  printf("[PASS] parser_comments\n");
}

static void test_parser_note(void) {
  char *ascii_output = render_fixture("tests/fixtures/notes.txt", RENDER_ASCII);
  char *expected_ascii = read_file("tests/fixtures/notes.ascii.out");
  assert(expected_ascii != NULL);
  assert(strcmp(ascii_output, expected_ascii) == 0);
  free(ascii_output);
  free(expected_ascii);

  char *utf8_output = render_fixture("tests/fixtures/notes.txt", RENDER_UTF8);
  char *expected_utf8 = read_file("tests/fixtures/notes.utf8.out");
  assert(expected_utf8 != NULL);
  assert(strcmp(utf8_output, expected_utf8) == 0);
  free(utf8_output);
  free(expected_utf8);

  printf("[PASS] parser_note\n");
}

static void test_parser_note_shorthand(void) {
  const char *input =
      "Alice->Bob: hello\nnote left: first\nnote right: second\nBob->Bob: "
      "self\nnote left\nmulti\nline\nend note\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Parser *p = parser_new(f);

  Diagram *d = parser_parse(p);
  assert(d != NULL);

  assert(d->participant_count == 2);
  assert(d->message_count == 2);
  assert(d->note_count == 3);
  assert(d->event_count == 2);

  assert(d->notes[0].position == NOTE_LEFT);
  assert(d->notes[0].from_idx == 0);
  assert(strcmp(d->notes[0].text, "first") == 0);

  assert(d->notes[1].position == NOTE_RIGHT);
  assert(d->notes[1].from_idx == 1);
  assert(strcmp(d->notes[1].text, "second") == 0);

  assert(d->notes[2].position == NOTE_LEFT);
  assert(d->notes[2].from_idx == 1);
  assert(strcmp(d->notes[2].text, "multi\nline") == 0);

  diagram_free(d);
  parser_free(p);
  fclose(f);
  printf("[PASS] parser_note_shorthand\n");
}

static void test_parser_note_requires_message(void) {
  const char *input = "note left: hello\n";
  FILE *f = open_memory_stream(input);
  assert(f != NULL);
  Parser *p = parser_new(f);

  Diagram *d = parser_parse(p);
  assert(d == NULL);
  assert(p->has_error);
  assert(p->error_msg != NULL);
  assert(strcmp(p->error_msg, "note left/right requires a preceding message") ==
         0);

  parser_free(p);
  fclose(f);
  printf("[PASS] parser_note_requires_message\n");
}

static void test_parser_complex_fixture(void) {
  const char *input_path = "tests/fixtures/complex-options.txt";

  char *ascii_output = render_fixture(input_path, RENDER_ASCII);
  char *expected_ascii = read_file("tests/fixtures/complex-options.ascii.out");
  assert(expected_ascii != NULL);
  assert(strcmp(ascii_output, expected_ascii) == 0);
  free(ascii_output);
  free(expected_ascii);

  char *utf8_output = render_fixture(input_path, RENDER_UTF8);
  char *expected_utf8 = read_file("tests/fixtures/complex-options.utf8.out");
  assert(expected_utf8 != NULL);
  assert(strcmp(utf8_output, expected_utf8) == 0);
  free(utf8_output);
  free(expected_utf8);

  printf("[PASS] parser_complex_fixture\n");
}

static void test_parser_complex_advanced_fixture(void) {
  const char *input_path = "tests/fixtures/complex-advanced.txt";

  char *ascii_output = render_fixture(input_path, RENDER_ASCII);
  char *expected_ascii = read_file("tests/fixtures/complex-advanced.ascii.out");
  assert(expected_ascii != NULL);
  assert(strcmp(ascii_output, expected_ascii) == 0);
  free(ascii_output);
  free(expected_ascii);

  char *utf8_output = render_fixture(input_path, RENDER_UTF8);
  char *expected_utf8 = read_file("tests/fixtures/complex-advanced.utf8.out");
  assert(expected_utf8 != NULL);
  assert(strcmp(utf8_output, expected_utf8) == 0);
  free(utf8_output);
  free(expected_utf8);

  printf("[PASS] parser_complex_advanced_fixture\n");
}

static void test_parser_stress_fixture(void) {
  const char *input_path = "tests/fixtures/stress-large.txt";

  char *ascii_output = render_fixture(input_path, RENDER_ASCII);
  char *expected_ascii = read_file("tests/fixtures/stress-large.ascii.out");
  assert(expected_ascii != NULL);
  assert(strcmp(ascii_output, expected_ascii) == 0);
  free(ascii_output);
  free(expected_ascii);

  char *utf8_output = render_fixture(input_path, RENDER_UTF8);
  char *expected_utf8 = read_file("tests/fixtures/stress-large.utf8.out");
  assert(expected_utf8 != NULL);
  assert(strcmp(utf8_output, expected_utf8) == 0);
  free(utf8_output);
  free(expected_utf8);

  printf("[PASS] parser_stress_fixture\n");
}

int main(void) {
  setlocale(LC_ALL, "");
  test_utf8_display_width();
  test_participant_new_free();
  test_message_new_free();
  test_diagram_lifecycle();
  test_lexer_arrows();
  test_lexer_participant();
  test_lexer_comments();
  test_lexer_message();
  test_lexer_multiline_participant_error();
  test_lexer_multiline_participant();
  test_parser_implicit_participants();
  test_parser_participant_conflict();
  test_parser_all_arrows();
  test_parser_self_message();
  test_parser_comments();
  test_parser_note_shorthand();
  test_parser_note_requires_message();
  test_parser_note();
  test_parser_complex_fixture();
  test_parser_complex_advanced_fixture();
  test_parser_stress_fixture();
  printf("All tests passed.\n");
  return 0;
}
