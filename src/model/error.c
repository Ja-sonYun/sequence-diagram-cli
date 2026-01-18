#include "error.h"
#include <stdio.h>

void seqdia_error_parse(int line, const char *reason) {
  if (!reason) {
    fprintf(stderr, "seqdia: parse error at line %d\n", line);
    return;
  }
  fprintf(stderr, "seqdia: parse error at line %d: %s\n", line, reason);
}

void seqdia_error_file(const char *path, const char *reason) {
  fprintf(stderr, "seqdia: file error: %s: %s\n", path, reason);
}

void seqdia_error_input(const char *reason) {
  fprintf(stderr, "seqdia: usage error: %s\n", reason);
}

void seqdia_error_memory(void) { fprintf(stderr, "seqdia: out of memory\n"); }
