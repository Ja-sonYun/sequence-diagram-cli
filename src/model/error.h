#ifndef SEQDIA_ERROR_H
#define SEQDIA_ERROR_H

typedef enum {
  SEQDIA_OK = 0,
  SEQDIA_ERR_PARSE = 1,
  SEQDIA_ERR_FILE = 2,
  SEQDIA_ERR_INPUT = 3,
  SEQDIA_ERR_MEMORY = 4
} SeqdiaError;

void seqdia_error_parse(int line, const char *reason);
void seqdia_error_file(const char *path, const char *reason);
void seqdia_error_input(const char *reason);
void seqdia_error_memory(void);

#endif
