#include "../model/error.h"
#include "../model/types.h"
#include "../parser/parser.h"
#include "../renderer/renderer.h"
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef SEQDIA_VERSION
#define SEQDIA_VERSION "dev"
#endif

static void print_version(void) { printf("seqdia %s\n", SEQDIA_VERSION); }

static void print_usage(void) {
  fprintf(stderr, "Usage: seqdia [options] [file]\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -v, --version             Print version\n");
  fprintf(stderr, "  -s, --style <ascii|utf8>  Output style (default: utf8)\n");
}

static bool parse_style(const char *value, RenderMode *mode) {
  if (strcmp(value, "ascii") == 0) {
    *mode = RENDER_ASCII;
    return true;
  }
  if (strcmp(value, "utf8") == 0) {
    *mode = RENDER_UTF8;
    return true;
  }
  return false;
}

int main(int argc, char *argv[]) {
  RenderMode mode = RENDER_UTF8;
  const char *filename = NULL;
  static char stdout_buffer[64 * 1024];
  setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));
  setlocale(LC_ALL, "");

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
      print_version();
      return 0;
    } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--style") == 0) {
      if (i + 1 >= argc) {
        seqdia_error_input("missing value for --style");
        print_usage();
        return SEQDIA_ERR_INPUT;
      }
      if (!parse_style(argv[i + 1], &mode)) {
        seqdia_error_input("unknown style");
        print_usage();
        return SEQDIA_ERR_INPUT;
      }
      i++;
    } else if (strncmp(argv[i], "-s=", 3) == 0) {
      const char *value = argv[i] + 3;
      if (*value == '\0') {
        seqdia_error_input("missing value for --style");
        print_usage();
        return SEQDIA_ERR_INPUT;
      }
      if (!parse_style(value, &mode)) {
        seqdia_error_input("unknown style");
        print_usage();
        return SEQDIA_ERR_INPUT;
      }
    } else if (strncmp(argv[i], "--style=", 8) == 0) {
      const char *value = argv[i] + 8;
      if (*value == '\0') {
        seqdia_error_input("missing value for --style");
        print_usage();
        return SEQDIA_ERR_INPUT;
      }
      if (!parse_style(value, &mode)) {
        seqdia_error_input("unknown style");
        print_usage();
        return SEQDIA_ERR_INPUT;
      }
    } else if (strcmp(argv[i], "-") == 0) {
      if (filename != NULL) {
        seqdia_error_input("multiple files specified");
        return SEQDIA_ERR_INPUT;
      }
      filename = "-";
    } else if (argv[i][0] == '-') {
      seqdia_error_input("unknown option");
      print_usage();
      return SEQDIA_ERR_INPUT;
    } else {
      if (filename != NULL) {
        seqdia_error_input("multiple files specified");
        return SEQDIA_ERR_INPUT;
      }
      filename = argv[i];
    }
  }

  FILE *input = NULL;
  bool should_close = false;
  bool has_stdin = !isatty(STDIN_FILENO);

  if (filename != NULL && strcmp(filename, "-") != 0) {
    input = fopen(filename, "r");
    if (!input) {
      seqdia_error_file(filename, "cannot open file");
      return SEQDIA_ERR_FILE;
    }
    should_close = true;
  } else if (filename != NULL || has_stdin) {
    input = stdin;
  } else {
    seqdia_error_input("no input specified");
    print_usage();
    return SEQDIA_ERR_INPUT;
  }

  Parser *parser = parser_new(input);
  if (!parser) {
    if (should_close)
      fclose(input);
    seqdia_error_memory();
    return SEQDIA_ERR_MEMORY;
  }

  Diagram *diagram = parser_parse(parser);

  if (!diagram) {
    if (parser->out_of_memory) {
      seqdia_error_memory();
      parser_free(parser);
      if (should_close)
        fclose(input);
      return SEQDIA_ERR_MEMORY;
    }
    if (parser->has_error) {
      seqdia_error_parse(parser->error_line, parser->error_msg);
    }
    parser_free(parser);
    if (should_close)
      fclose(input);
    return SEQDIA_ERR_PARSE;
  }

  if (!render_diagram(diagram, mode, stdout)) {
    diagram_free(diagram);
    parser_free(parser);
    if (should_close)
      fclose(input);
    seqdia_error_memory();
    return SEQDIA_ERR_MEMORY;
  }

  diagram_free(diagram);
  parser_free(parser);
  if (should_close)
    fclose(input);

  return 0;
}
