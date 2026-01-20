#include "canvas.h"
#include "../model/types.h"
#include <stdlib.h>
#include <string.h>

Canvas *canvas_new(size_t width, size_t height) {
  Canvas *c = malloc(sizeof(Canvas));
  if (!c)
    return NULL;

  c->width = width;
  c->height = height;
  c->data = malloc(width * height * CELL_SIZE);
  if (!c->data) {
    free(c);
    return NULL;
  }

  for (size_t i = 0; i < width * height; i++) {
    char *cell = &c->data[i * CELL_SIZE];
    cell[0] = ' ';
    cell[1] = '\0';
  }

  return c;
}

void canvas_free(Canvas *c) {
  if (!c)
    return;
  free(c->data);
  free(c);
}

char *canvas_cell(Canvas *c, size_t x, size_t y) {
  if (!c || x >= c->width || y >= c->height)
    return NULL;
  return &c->data[(y * c->width + x) * CELL_SIZE];
}

const char *canvas_cell_const(const Canvas *c, size_t x, size_t y) {
  if (!c || x >= c->width || y >= c->height)
    return NULL;
  return &c->data[(y * c->width + x) * CELL_SIZE];
}

void canvas_put(Canvas *c, size_t x, size_t y, const char *ch) {
  char *cell = canvas_cell(c, x, y);
  if (!cell || !ch)
    return;

  size_t len = strlen(ch);
  if (len >= CELL_SIZE)
    len = CELL_SIZE - 1;
  memcpy(cell, ch, len);
  cell[len] = '\0';
}

static size_t utf8_char_len(unsigned char c) {
  if ((c & 0x80) == 0)
    return 1;
  if ((c & 0xE0) == 0xC0)
    return 2;
  if ((c & 0xF0) == 0xE0)
    return 3;
  if ((c & 0xF8) == 0xF0)
    return 4;
  return 1;
}

void canvas_puts(Canvas *c, size_t x, size_t y, const char *str) {
  if (!str)
    return;
  canvas_puts_n(c, x, y, str, strlen(str));
}

void canvas_puts_n(Canvas *c, size_t x, size_t y, const char *str, size_t len) {
  if (!c || !str || len == 0)
    return;

  size_t col = x;
  const unsigned char *p = (const unsigned char *)str;
  const unsigned char *end = p + len;

  while (p < end && col < c->width) {
    size_t char_len = utf8_char_len(*p);
    if (p + char_len > end)
      break;

    size_t display_width = utf8_display_width_n((const char *)p, char_len);

    char *cell = canvas_cell(c, col, y);
    if (cell && char_len < CELL_SIZE) {
      memcpy(cell, p, char_len);
      cell[char_len] = '\0';
    }

    if (display_width == 2 && col + 1 < c->width) {
      char *next = canvas_cell(c, col + 1, y);
      if (next) {
        next[0] = WIDE_MARKER;
        next[1] = '\0';
      }
    }

    col += display_width;
    p += char_len;
  }
}

void canvas_hline(Canvas *c, size_t x1, size_t x2, size_t y, const char *ch) {
  if (!c || x1 > x2)
    return;
  for (size_t x = x1; x <= x2 && x < c->width; x++) {
    canvas_put(c, x, y, ch);
  }
}

void canvas_vline(Canvas *c, size_t x, size_t y1, size_t y2, const char *ch) {
  if (!c || y1 > y2)
    return;
  for (size_t y = y1; y <= y2 && y < c->height; y++) {
    canvas_put(c, x, y, ch);
  }
}

int canvas_print(Canvas *c, FILE *output) {
  if (!c || !output)
    return -1;

  for (size_t y = 0; y < c->height; y++) {
    size_t last_content = 0;
    for (size_t x = 0; x < c->width; x++) {
      const char *cell = canvas_cell_const(c, x, y);
      if (cell && cell[0] != ' ' && cell[0] != WIDE_MARKER) {
        last_content = x + 1;
      }
    }

    for (size_t x = 0; x < last_content; x++) {
      const char *cell = canvas_cell_const(c, x, y);
      if (!cell)
        continue;

      if (cell[0] == WIDE_MARKER)
        continue;

      if (fputs(cell, output) == EOF)
        return -1;
    }

    if (fputc('\n', output) == EOF)
      return -1;
  }

  return 0;
}
