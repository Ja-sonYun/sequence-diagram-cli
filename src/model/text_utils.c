#include "text_utils.h"
#include "types.h"
#include <string.h>

size_t count_lines(const char *text) {
  if (!text || !*text)
    return 1;
  size_t count = 1;
  for (const char *p = text; *p; p++) {
    if (*p == '\n')
      count++;
  }
  return count;
}

size_t max_line_width(const char *text) {
  if (!text || !*text)
    return 1;

  size_t max_width = 0;
  const char *line_start = text;
  const char *p = text;

  while (*p) {
    if (*p == '\n') {
      size_t line_len = (size_t)(p - line_start);
      size_t line_width = utf8_display_width_n(line_start, line_len);
      if (line_width > max_width)
        max_width = line_width;
      line_start = p + 1;
    }
    p++;
  }

  size_t line_len = (size_t)(p - line_start);
  size_t line_width = utf8_display_width_n(line_start, line_len);
  if (line_width > max_width)
    max_width = line_width;

  return max_width;
}
