#ifndef SEQDIA_CANVAS_H
#define SEQDIA_CANVAS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CELL_SIZE 5
#define WIDE_MARKER '\x01'

typedef struct {
  char *data;
  size_t width;
  size_t height;
} Canvas;

Canvas *canvas_new(size_t width, size_t height);
void canvas_free(Canvas *c);

char *canvas_cell(Canvas *c, size_t x, size_t y);
const char *canvas_cell_const(const Canvas *c, size_t x, size_t y);

void canvas_put(Canvas *c, size_t x, size_t y, const char *ch);
void canvas_puts(Canvas *c, size_t x, size_t y, const char *str);
void canvas_puts_n(Canvas *c, size_t x, size_t y, const char *str, size_t len);

void canvas_hline(Canvas *c, size_t x1, size_t x2, size_t y, const char *ch);
void canvas_vline(Canvas *c, size_t x, size_t y1, size_t y2, const char *ch);

int canvas_print(Canvas *c, FILE *output);

#endif
