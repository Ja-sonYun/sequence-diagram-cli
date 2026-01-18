#include "renderer.h"
#include "../model/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const RenderChars UTF8_CHARS = {
    .box_tl = "╭",
    .box_tr = "╮",
    .box_bl = "╰",
    .box_br = "╯",
    .box_h = "─",
    .box_v = "│",
    .box_bt = "┬",
    .life_v = "┆",
    .life_branch_left = "├",
    .life_branch_right = "┤",
    .arrow_right = "▶",
    .arrow_left = "◀",
    .arrow_right_open = "▷",
    .arrow_left_open = "◁",
    .arrow_right_dashed = "≻",
    .arrow_left_dashed = "≺",
    .arrow_right_x = "x",
    .arrow_left_x = "x",
    .line_solid = "─",
    .line_dashed = "╶",
};

static const RenderChars ASCII_CHARS = {
    .box_tl = ",",
    .box_tr = ".",
    .box_bl = "`",
    .box_br = "'",
    .box_h = "-",
    .box_v = "|",
    .box_bt = "-",
    .life_v = "|",
    .life_branch_left = "+",
    .life_branch_right = "+",
    .arrow_right = ">",
    .arrow_left = "<",
    .arrow_right_open = ")",
    .arrow_left_open = "(",
    .arrow_right_dashed = ">",
    .arrow_left_dashed = "<",
    .arrow_right_x = "x",
    .arrow_left_x = "x",
    .line_solid = "-",
    .line_dashed = "~",
};

typedef struct {
  size_t *positions;
  size_t *widths;
  size_t count;
  size_t total_width;
} Layout;

static const char *get_line(const char *text, size_t line_idx, size_t *len) {
  if (!text) {
    *len = 0;
    return NULL;
  }

  const char *start = text;
  size_t idx = 0;

  while (idx < line_idx && *start) {
    if (*start == '\n')
      idx++;
    start++;
  }

  if (!*start && idx < line_idx) {
    *len = 0;
    return NULL;
  }

  const char *end = start;
  while (*end && *end != '\n')
    end++;

  *len = end - start;
  return start;
}

static void ensure_note_span(Layout *layout, size_t left, size_t right,
                             size_t note_width) {
  size_t span = layout->widths[left] / 2 + layout->widths[right] / 2 + 2;
  for (size_t j = left + 1; j < right; j++) {
    span += layout->widths[j] + 2;
  }
  size_t available = span > 0 ? span - 1 : 0;
  if (note_width > available) {
    size_t extra = note_width - available;
    size_t left_extra = (extra + 1) / 2;
    size_t right_extra = extra / 2;
    layout->widths[left] += left_extra * 2;
    layout->widths[right] += right_extra * 2;
  }
}

static Layout *compute_layout(Diagram *d) {
  Layout *layout = malloc(sizeof(Layout));
  if (!layout)
    return NULL;

  layout->count = d->participant_count;
  layout->positions = calloc(layout->count, sizeof(size_t));
  layout->widths = calloc(layout->count, sizeof(size_t));
  if (!layout->positions || !layout->widths) {
    free(layout->positions);
    free(layout->widths);
    free(layout);
    return NULL;
  }

  for (size_t i = 0; i < d->participant_count; i++) {
    const Participant *participant = &d->participants[i];
    size_t name_width = participant->max_line_width;
    size_t name_lines = participant->line_count;
    size_t min_width = name_lines > 1 ? 3 : 7;
    layout->widths[i] = name_width + 4;
    if (layout->widths[i] < min_width)
      layout->widths[i] = min_width;
  }

  for (size_t i = 0; i < d->message_count; i++) {
    Message *m = &d->messages[i];
    if (m->from_idx == m->to_idx) {
      size_t loop_width = max_line_width(m->text) + 4;
      size_t required_span = loop_width + 2;
      size_t idx = m->from_idx;
      if (idx + 1 < d->participant_count) {
        size_t current_span =
            layout->widths[idx] / 2 + layout->widths[idx + 1] / 2 + 2;
        if (required_span > current_span) {
          size_t extra = required_span - current_span;
          layout->widths[idx + 1] += extra * 2;
        }
      } else {
        size_t current_span = layout->widths[idx] / 2 + 2;
        if (required_span > current_span) {
          size_t extra = required_span - current_span;
          layout->widths[idx] += extra * 2;
        }
      }
      continue;
    }

    size_t left = m->from_idx < m->to_idx ? m->from_idx : m->to_idx;
    size_t right = m->from_idx < m->to_idx ? m->to_idx : m->from_idx;

    size_t msg_width = max_line_width(m->text) + 6;

    size_t current_span =
        layout->widths[left] / 2 + layout->widths[right] / 2 + 2;
    for (size_t j = left + 1; j < right; j++) {
      current_span += layout->widths[j] + 2;
    }

    size_t available = current_span > 0 ? current_span - 1 : 0;
    if (msg_width > available) {
      size_t extra = msg_width - available;
      size_t left_extra = (extra + 1) / 2;
      size_t right_extra = extra / 2;
      layout->widths[left] += left_extra * 2;
      layout->widths[right] += right_extra * 2;
    }
  }

  for (size_t i = 0; i < d->note_count; i++) {
    Note *note = &d->notes[i];
    size_t note_width = note->max_line_width + 4;
    size_t left = note->from_idx;
    size_t right = note->to_idx;
    if (note->position == NOTE_LEFT) {
      if (left > 0) {
        ensure_note_span(layout, left - 1, left, note_width + 2);
      } else {
        layout->widths[left] += note_width * 2;
      }
    } else if (note->position == NOTE_RIGHT) {
      if (right + 1 < d->participant_count) {
        ensure_note_span(layout, right, right + 1, note_width + 2);
      } else {
        layout->widths[right] += note_width * 2;
      }
    } else {
      ensure_note_span(layout, left, right, note_width + 2);
    }
  }

  size_t pos = 0;
  for (size_t i = 0; i < layout->count; i++) {
    layout->positions[i] = pos + layout->widths[i] / 2;
    pos += layout->widths[i] + 2;
  }
  layout->total_width = pos;

  return layout;
}

static void layout_free(Layout *layout) {
  if (!layout)
    return;
  free(layout->positions);
  free(layout->widths);
  free(layout);
}

static int print_spaces_safe(FILE *output, size_t from, size_t to) {
  if (to > from) {
    for (size_t i = 0; i < to - from; i++) {
      if (fputc(' ', output) == EOF)
        return -1;
    }
  }
  return 0;
}

static int print_spaces(FILE *output, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (fputc(' ', output) == EOF)
      return -1;
  }
  return 0;
}

static int print_repeat(FILE *output, const char *s, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (fputs(s, output) == EOF)
      return -1;
  }
  return 0;
}

static int print_bytes(FILE *output, const char *text, size_t len) {
  if (len == 0)
    return 0;
  if (fwrite(text, 1, len, output) != len)
    return -1;
  return 0;
}

static int render_lifeline(FILE *output, Diagram *d, Layout *layout,
                           const RenderChars *chars) {
  size_t col = 0;
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t pos = layout->positions[i];
    if (print_spaces(output, pos - col) < 0)
      return -1;
    if (fputs(chars->life_v, output) == EOF)
      return -1;
    col = pos + 1;
  }
  if (print_spaces(output, layout->total_width - col) < 0)
    return -1;
  if (fputc('\n', output) == EOF)
    return -1;
  return 0;
}

static int render_header_top(FILE *output, Diagram *d, Layout *layout,
                             const RenderChars *chars) {
  size_t col = 0;
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t pos = layout->positions[i];
    size_t width = layout->widths[i];
    size_t box_start = pos - width / 2;

    if (print_spaces(output, box_start - col) < 0)
      return -1;
    if (fputs(chars->box_tl, output) == EOF)
      return -1;
    if (print_repeat(output, chars->box_h, width - 2) < 0)
      return -1;
    if (fputs(chars->box_tr, output) == EOF)
      return -1;
    col = box_start + width;
  }
  if (print_spaces(output, layout->total_width - col) < 0)
    return -1;
  if (fputc('\n', output) == EOF)
    return -1;
  return 0;
}

static int render_header_name(FILE *output, Diagram *d, Layout *layout,
                              const RenderChars *chars) {
  size_t max_lines = 1;
  for (size_t i = 0; i < d->participant_count; i++) {
    if (d->participants[i].line_count > max_lines)
      max_lines = d->participants[i].line_count;
  }

  for (size_t line_idx = 0; line_idx < max_lines; line_idx++) {
    size_t col = 0;
    for (size_t i = 0; i < d->participant_count; i++) {
      size_t pos = layout->positions[i];
      size_t width = layout->widths[i];
      size_t box_start = pos - width / 2;

      if (print_spaces(output, box_start - col) < 0)
        return -1;
      if (fputs(chars->box_v, output) == EOF)
        return -1;

      const Participant *participant = &d->participants[i];
      size_t inner = width - 2;
      size_t text_len = 0;
      const char *text = get_line(participant->name, line_idx, &text_len);
      size_t text_width = 0;
      if (text) {
        text_width = utf8_display_width_n(text, text_len);
      }
      size_t pad_left = (inner - text_width) / 2;
      size_t pad_right = inner - text_width - pad_left;

      if (print_spaces(output, pad_left) < 0)
        return -1;
      if (text) {
        if (print_bytes(output, text, text_len) < 0)
          return -1;
      }
      if (print_spaces(output, pad_right) < 0)
        return -1;
      if (fputs(chars->box_v, output) == EOF)
        return -1;
      col = box_start + width;
    }
    if (print_spaces(output, layout->total_width - col) < 0)
      return -1;
    if (fputc('\n', output) == EOF)
      return -1;
  }
  return 0;
}

static int render_header_bottom(FILE *output, Diagram *d, Layout *layout,
                                const RenderChars *chars) {
  size_t col = 0;
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t pos = layout->positions[i];
    size_t width = layout->widths[i];
    size_t box_start = pos - width / 2;
    size_t half = width / 2;

    if (print_spaces(output, box_start - col) < 0)
      return -1;
    if (fputs(chars->box_bl, output) == EOF)
      return -1;
    if (print_repeat(output, chars->box_h, half - 1) < 0)
      return -1;
    if (fputs(chars->box_bt, output) == EOF)
      return -1;
    if (print_repeat(output, chars->box_h, width - 2 - half) < 0)
      return -1;
    if (fputs(chars->box_br, output) == EOF)
      return -1;
    col = box_start + width;
  }
  if (print_spaces(output, layout->total_width - col) < 0)
    return -1;
  if (fputc('\n', output) == EOF)
    return -1;

  size_t max_lines = 1;
  for (size_t i = 0; i < d->participant_count; i++) {
    if (d->participants[i].line_count > max_lines)
      max_lines = d->participants[i].line_count;
  }

  for (size_t line_idx = 1; line_idx < max_lines; line_idx++) {
    if (render_lifeline(output, d, layout, chars) < 0)
      return -1;
  }
  return 0;
}

static int render_self_message(FILE *output, Diagram *d, Layout *layout,
                               Message *m, const RenderChars *chars) {
  size_t msg_width = max_line_width(m->text);
  size_t loop_width = msg_width + 4;
  size_t num_lines = count_lines(m->text);

  bool is_dashed = (m->arrow == ARROW_DASHED || m->arrow == ARROW_DASHED_LEFT);
  bool is_x = (m->arrow == ARROW_X || m->arrow == ARROW_X_LEFT);
  const char *line_char = is_dashed ? chars->line_dashed : chars->line_solid;
  const char *arrow_char =
      is_dashed ? chars->arrow_left_dashed
                : (is_x ? chars->arrow_left_x : chars->arrow_left);

  size_t col;

  if (render_lifeline(output, d, layout, chars) < 0)
    return -1;

  for (size_t ln = 0; ln < num_lines; ln++) {
    size_t text_len;
    const char *text = get_line(m->text, ln, &text_len);
    size_t text_width = utf8_display_width_n(text, text_len);
    size_t text_start = layout->positions[m->from_idx] + 2;
    size_t text_end = text_start + text_width;
    bool text_printed = false;

    col = 0;
    for (size_t i = 0; i < d->participant_count; i++) {
      size_t p = layout->positions[i];
      if (!text_printed && text_start <= p) {
        if (print_spaces_safe(output, col, text_start) < 0)
          return -1;
        if (print_bytes(output, text, text_len) < 0)
          return -1;
        col = text_end;
        text_printed = true;
      }
      if (p >= text_start && p < text_end) {
        continue;
      }
      if (print_spaces_safe(output, col, p) < 0)
        return -1;
      if (fputs(chars->life_v, output) == EOF)
        return -1;
      col = p + 1;
    }
    if (!text_printed) {
      if (print_spaces_safe(output, col, text_start) < 0)
        return -1;
      if (print_bytes(output, text, text_len) < 0)
        return -1;
    }
    if (fputc('\n', output) == EOF)
      return -1;
  }

  size_t loop_start = layout->positions[m->from_idx] + 1;
  size_t loop_end = loop_start + loop_width + 1;

  col = 0;
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t p = layout->positions[i];
    if (p > loop_start && p < loop_end) {
      continue;
    }
    if (print_spaces_safe(output, col, p) < 0)
      return -1;
    if (i == m->from_idx) {
      if (fputs(chars->life_branch_left, output) == EOF)
        return -1;
      if (print_repeat(output, line_char, loop_width) < 0)
        return -1;
      if (fputs(chars->box_tr, output) == EOF)
        return -1;
      col = loop_end;
    } else {
      if (fputs(chars->life_v, output) == EOF)
        return -1;
      col = p + 1;
    }
  }
  if (fputc('\n', output) == EOF)
    return -1;

  col = 0;
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t p = layout->positions[i];
    if (p > loop_start && p < loop_end) {
      continue;
    }
    if (print_spaces_safe(output, col, p) < 0)
      return -1;
    if (i == m->from_idx) {
      if (fputs(chars->life_v, output) == EOF)
        return -1;
      if (print_spaces(output, loop_width) < 0)
        return -1;
      if (fputs(chars->life_v, output) == EOF)
        return -1;
      col = loop_end;
    } else {
      if (fputs(chars->life_v, output) == EOF)
        return -1;
      col = p + 1;
    }
  }
  if (fputc('\n', output) == EOF)
    return -1;

  col = 0;
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t p = layout->positions[i];
    if (p > loop_start && p < loop_end) {
      continue;
    }
    if (print_spaces_safe(output, col, p) < 0)
      return -1;
    if (i == m->from_idx) {
      if (fputs(arrow_char, output) == EOF)
        return -1;
      if (print_repeat(output, line_char, loop_width) < 0)
        return -1;
      if (fputs(chars->box_br, output) == EOF)
        return -1;
      col = loop_end;
    } else {
      if (fputs(chars->life_v, output) == EOF)
        return -1;
      col = p + 1;
    }
  }
  if (fputc('\n', output) == EOF)
    return -1;
  return 0;
}

static size_t calc_note_box_start(Layout *layout, Note *note,
                                  size_t box_width) {
  size_t left_pos = layout->positions[note->from_idx];
  size_t right_pos = layout->positions[note->to_idx];

  if (note->position == NOTE_LEFT) {
    return left_pos > box_width + 2 ? left_pos - box_width - 2 : 0;
  }
  if (note->position == NOTE_RIGHT) {
    return right_pos + 2;
  }

  size_t span_left = left_pos;
  size_t span_right = right_pos;
  size_t span_width = span_right - span_left;
  if (span_width > box_width)
    return span_left + (span_width - box_width) / 2;
  if (span_left > 0) {
    size_t offset = (box_width - span_width) / 2;
    return span_left > offset ? span_left - offset : 0;
  }
  return 0;
}

static int render_note_box_line(FILE *output, Diagram *d, Layout *layout,
                                size_t box_start, size_t box_end,
                                const RenderChars *chars, const char *left_edge,
                                const char *right_edge, const char *text,
                                size_t text_len) {
  size_t col = 0;
  bool box_printed = false;
  size_t inner_width = box_end - box_start - 2;
  size_t text_width = text ? utf8_display_width_n(text, text_len) : 0;
  size_t pad_left = 0;
  size_t pad_right = 0;

  if (text) {
    pad_left = (inner_width - text_width) / 2;
    pad_right = inner_width - text_width - pad_left;
  }

  for (size_t i = 0; i < d->participant_count; i++) {
    size_t pos = layout->positions[i];
    if (!box_printed && box_start <= pos) {
      if (print_spaces_safe(output, col, box_start) < 0)
        return -1;
      if (fputs(left_edge, output) == EOF)
        return -1;
      if (text) {
        if (print_spaces(output, pad_left) < 0)
          return -1;
        if (print_bytes(output, text, text_len) < 0)
          return -1;
        if (print_spaces(output, pad_right) < 0)
          return -1;
      } else {
        if (print_repeat(output, chars->box_h, inner_width) < 0)
          return -1;
      }
      if (fputs(right_edge, output) == EOF)
        return -1;
      col = box_end;
      box_printed = true;
    }

    if (pos >= box_start && pos < box_end)
      continue;
    if (print_spaces_safe(output, col, pos) < 0)
      return -1;
    if (fputs(chars->life_v, output) == EOF)
      return -1;
    col = pos + 1;
  }

  if (!box_printed) {
    if (print_spaces_safe(output, col, box_start) < 0)
      return -1;
    if (fputs(left_edge, output) == EOF)
      return -1;
    if (text) {
      if (print_spaces(output, pad_left) < 0)
        return -1;
      if (print_bytes(output, text, text_len) < 0)
        return -1;
      if (print_spaces(output, pad_right) < 0)
        return -1;
    } else {
      if (print_repeat(output, chars->box_h, inner_width) < 0)
        return -1;
    }
    if (fputs(right_edge, output) == EOF)
      return -1;
    col = box_end;
  }

  if (layout->total_width > col) {
    if (print_spaces(output, layout->total_width - col) < 0)
      return -1;
  }
  if (fputc('\n', output) == EOF)
    return -1;
  return 0;
}

static int render_note(FILE *output, Diagram *d, Layout *layout, Note *note,
                       const RenderChars *chars) {
  size_t note_width = note->max_line_width + 2;
  size_t box_width = note_width + 2;
  size_t box_start = calc_note_box_start(layout, note, box_width);
  size_t box_end = box_start + box_width;

  if (render_note_box_line(output, d, layout, box_start, box_end, chars,
                           chars->box_tl, chars->box_tr, NULL, 0) < 0)
    return -1;

  for (size_t line_idx = 0; line_idx < note->line_count; line_idx++) {
    size_t text_len = 0;
    const char *text = get_line(note->text, line_idx, &text_len);
    if (render_note_box_line(output, d, layout, box_start, box_end, chars,
                             chars->box_v, chars->box_v, text, text_len) < 0)
      return -1;
  }

  if (render_note_box_line(output, d, layout, box_start, box_end, chars,
                           chars->box_bl, chars->box_br, NULL, 0) < 0)
    return -1;
  return 0;
}

static int render_inline_note_line(FILE *output, size_t *col, size_t box_start,
                                   size_t box_end, const RenderChars *chars,
                                   const char *left_edge,
                                   const char *right_edge, const char *text,
                                   size_t text_len) {
  size_t inner_width = box_end - box_start - 2;
  size_t text_width = text ? utf8_display_width_n(text, text_len) : 0;
  size_t pad_left = 0;
  size_t pad_right = 0;

  if (text) {
    pad_left = (inner_width - text_width) / 2;
    pad_right = inner_width - text_width - pad_left;
  }

  if (print_spaces_safe(output, *col, box_start) < 0)
    return -1;
  if (fputs(left_edge, output) == EOF)
    return -1;
  if (text) {
    if (print_spaces(output, pad_left) < 0)
      return -1;
    if (print_bytes(output, text, text_len) < 0)
      return -1;
    if (print_spaces(output, pad_right) < 0)
      return -1;
  } else {
    if (print_repeat(output, chars->box_h, inner_width) < 0)
      return -1;
  }
  if (fputs(right_edge, output) == EOF)
    return -1;
  *col = box_end;
  return 0;
}

static int render_inline_row(
    FILE *output, Diagram *d, Layout *layout, const RenderChars *chars,
    bool draw_arrow, bool left_to_right, size_t from_pos, size_t left_pos,
    size_t right_pos, const char *line_char, const char *arrow_char,
    size_t line_span, bool left_note, size_t left_box_start,
    size_t left_box_end, const char *left_edge, const char *left_right_edge,
    const char *left_text, size_t left_text_len, bool right_note,
    size_t right_box_start, size_t right_box_end, const char *right_left_edge,
    const char *right_right_edge, const char *right_text,
    size_t right_text_len) {
  size_t col = 0;
  bool left_drawn = false;
  bool right_drawn = false;

  for (size_t i = 0; i < d->participant_count; i++) {
    size_t pos = layout->positions[i];

    if (left_note && !left_drawn && left_box_start <= pos) {
      if (render_inline_note_line(output, &col, left_box_start, left_box_end,
                                  chars, left_edge, left_right_edge, left_text,
                                  left_text_len) < 0)
        return -1;
      left_drawn = true;
      while (i + 1 < d->participant_count &&
             layout->positions[i + 1] < left_box_end)
        i++;
      if (pos < left_box_end)
        continue;
    }

    if (right_note && !right_drawn && right_box_start <= pos) {
      if (render_inline_note_line(output, &col, right_box_start, right_box_end,
                                  chars, right_left_edge, right_right_edge,
                                  right_text, right_text_len) < 0)
        return -1;
      right_drawn = true;
      while (i + 1 < d->participant_count &&
             layout->positions[i + 1] < right_box_end)
        i++;
      if (pos < right_box_end)
        continue;
    }

    if (print_spaces_safe(output, col, pos) < 0)
      return -1;
    if (draw_arrow) {
      if (left_to_right) {
        if (pos == from_pos) {
          if (fputs(chars->life_branch_left, output) == EOF)
            return -1;
          if (line_span > 0) {
            if (print_repeat(output, line_char, line_span) < 0)
              return -1;
          }
          if (fputs(arrow_char, output) == EOF)
            return -1;
          col = right_pos;
          while (i + 1 < d->participant_count &&
                 layout->positions[i + 1] < right_pos)
            i++;
        } else {
          if (fputs(chars->life_v, output) == EOF)
            return -1;
          col = pos + 1;
        }
      } else {
        if (pos == left_pos) {
          if (fputs(chars->life_v, output) == EOF)
            return -1;
          if (fputs(arrow_char, output) == EOF)
            return -1;
          if (line_span > 0) {
            if (print_repeat(output, line_char, line_span) < 0)
              return -1;
          }
          if (fputs(chars->life_branch_right, output) == EOF)
            return -1;
          col = right_pos + 1;
          while (i + 1 < d->participant_count &&
                 layout->positions[i + 1] <= right_pos)
            i++;
        } else {
          if (fputs(chars->life_v, output) == EOF)
            return -1;
          col = pos + 1;
        }
      }
    } else {
      if (fputs(chars->life_v, output) == EOF)
        return -1;
      col = pos + 1;
    }
  }

  if (left_note && !left_drawn) {
    if (render_inline_note_line(output, &col, left_box_start, left_box_end,
                                chars, left_edge, left_right_edge, left_text,
                                left_text_len) < 0)
      return -1;
  }
  if (right_note && !right_drawn) {
    if (render_inline_note_line(output, &col, right_box_start, right_box_end,
                                chars, right_left_edge, right_right_edge,
                                right_text, right_text_len) < 0)
      return -1;
  }

  if (fputc('\n', output) == EOF)
    return -1;
  return 0;
}

static int render_message_text_line(
    FILE *output, Diagram *d, Layout *layout, const RenderChars *chars,
    const char *text, size_t text_len, size_t text_start, size_t text_end,
    bool left_note, size_t left_box_start, size_t left_box_end,
    const char *left_edge, const char *left_right_edge, bool right_note,
    size_t right_box_start, size_t right_box_end, const char *right_left_edge,
    const char *right_right_edge) {
  size_t col = 0;
  bool text_printed = false;
  bool left_drawn = false;
  bool right_drawn = false;

  for (size_t i = 0; i < d->participant_count; i++) {
    size_t pos = layout->positions[i];

    if (left_note && !left_drawn && left_box_start <= pos) {
      if (render_inline_note_line(output, &col, left_box_start, left_box_end,
                                  chars, left_edge, left_right_edge, NULL,
                                  0) < 0)
        return -1;
      left_drawn = true;
      while (i + 1 < d->participant_count &&
             layout->positions[i + 1] < left_box_end)
        i++;
      if (pos < left_box_end)
        continue;
    }

    if (!text_printed && text_start <= pos) {
      if (print_spaces_safe(output, col, text_start) < 0)
        return -1;
      if (print_bytes(output, text, text_len) < 0)
        return -1;
      col = text_end;
      text_printed = true;
    }

    if (right_note && !right_drawn && right_box_start <= pos) {
      if (render_inline_note_line(output, &col, right_box_start, right_box_end,
                                  chars, right_left_edge, right_right_edge,
                                  NULL, 0) < 0)
        return -1;
      right_drawn = true;
      while (i + 1 < d->participant_count &&
             layout->positions[i + 1] < right_box_end)
        i++;
      if (pos < right_box_end)
        continue;
    }

    if (pos >= text_start && pos < text_end) {
      continue;
    }

    if (print_spaces_safe(output, col, pos) < 0)
      return -1;
    if (fputs(chars->life_v, output) == EOF)
      return -1;
    col = pos + 1;
  }

  if (left_note && !left_drawn) {
    if (render_inline_note_line(output, &col, left_box_start, left_box_end,
                                chars, left_edge, left_right_edge, NULL, 0) < 0)
      return -1;
  }
  if (right_note && !right_drawn) {
    if (render_inline_note_line(output, &col, right_box_start, right_box_end,
                                chars, right_left_edge, right_right_edge, NULL,
                                0) < 0)
      return -1;
  }
  if (!text_printed) {
    if (print_spaces_safe(output, col, text_start) < 0)
      return -1;
    if (print_bytes(output, text, text_len) < 0)
      return -1;
  }

  if (fputc('\n', output) == EOF)
    return -1;
  return 0;
}

static int render_message(FILE *output, Diagram *d, Layout *layout, Message *m,
                          const RenderChars *chars) {
  size_t from_pos = layout->positions[m->from_idx];
  size_t to_pos = layout->positions[m->to_idx];

  if (m->is_self) {
    return render_self_message(output, d, layout, m, chars);
  }

  bool left_to_right = from_pos < to_pos;
  size_t left_pos = left_to_right ? from_pos : to_pos;
  size_t right_pos = left_to_right ? to_pos : from_pos;

  size_t line_width = right_pos - left_pos - 1;
  size_t num_lines = count_lines(m->text);

  Note *left_note = NULL;
  Note *right_note = NULL;

  if (m->inline_left_note_idx != (size_t)-1) {
    left_note = &d->notes[m->inline_left_note_idx];
  }
  if (m->inline_right_note_idx != (size_t)-1) {
    right_note = &d->notes[m->inline_right_note_idx];
  }

  size_t left_box_start = 0;
  size_t left_box_end = 0;
  size_t right_box_start = 0;
  size_t right_box_end = 0;

  if (left_note) {
    size_t note_width = left_note->max_line_width + 2;
    size_t box_width = note_width + 2;
    left_box_start = left_pos > box_width + 2 ? left_pos - box_width - 2 : 0;
    left_box_end = left_box_start + box_width;
  }

  if (right_note) {
    size_t note_width = right_note->max_line_width + 2;
    size_t box_width = note_width + 2;
    right_box_start = right_pos + 2;
    right_box_end = right_box_start + box_width;
  }

  size_t col = 0;
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t pos = layout->positions[i];
    if (print_spaces_safe(output, col, pos) < 0)
      return -1;
    if (fputs(chars->life_v, output) == EOF)
      return -1;
    col = pos + 1;
  }
  if (fputc('\n', output) == EOF)
    return -1;

  for (size_t ln = 0; ln < num_lines; ln++) {
    size_t text_len;
    const char *text = get_line(m->text, ln, &text_len);
    size_t text_width = utf8_display_width_n(text, text_len);
    size_t msg_pad =
        (line_width > text_width) ? (line_width - text_width) / 2 : 0;
    size_t text_start = left_pos + 1 + msg_pad;
    size_t text_end = text_start + text_width;
    bool draw_top = (ln + 1 == num_lines);

    if (m->inline_left_note_idx == (size_t)-1 &&
        m->inline_right_note_idx == (size_t)-1) {
      col = 0;
      bool text_printed = false;
      for (size_t i = 0; i < d->participant_count; i++) {
        size_t pos = layout->positions[i];
        if (!text_printed && text_start <= pos) {
          if (print_spaces_safe(output, col, text_start) < 0)
            return -1;
          if (print_bytes(output, text, text_len) < 0)
            return -1;
          col = text_end;
          text_printed = true;
        }
        if (pos >= text_start && pos < text_end) {
          continue;
        }
        if (print_spaces_safe(output, col, pos) < 0)
          return -1;
        if (fputs(chars->life_v, output) == EOF)
          return -1;
        col = pos + 1;
      }
      if (!text_printed) {
        if (print_spaces_safe(output, col, text_start) < 0)
          return -1;
        if (print_bytes(output, text, text_len) < 0)
          return -1;
      }
      if (fputc('\n', output) == EOF)
        return -1;
      continue;
    }

    if (render_message_text_line(
            output, d, layout, chars, text, text_len, text_start, text_end,
            draw_top ? left_note : false, left_box_start, left_box_end,
            draw_top ? chars->box_tl : chars->box_v,
            draw_top ? chars->box_tr : chars->box_v,
            draw_top ? right_note : false, right_box_start, right_box_end,
            draw_top ? chars->box_tl : chars->box_v,
            draw_top ? chars->box_tr : chars->box_v) < 0)
      return -1;
  }

  bool is_dashed = (m->arrow == ARROW_DASHED || m->arrow == ARROW_DASHED_LEFT);
  bool is_open = (m->arrow == ARROW_OPEN || m->arrow == ARROW_OPEN_LEFT);
  bool is_x = (m->arrow == ARROW_X || m->arrow == ARROW_X_LEFT);
  const char *line_char = is_dashed ? chars->line_dashed : chars->line_solid;
  const char *arrow_char;
  size_t line_span = line_width > 1 ? line_width - 1 : 0;

  if (left_to_right) {
    if (is_dashed) {
      arrow_char = chars->arrow_right_dashed;
    } else if (is_open) {
      arrow_char = chars->arrow_right_open;
    } else if (is_x) {
      arrow_char = chars->arrow_right_x;
    } else {
      arrow_char = chars->arrow_right;
    }
  } else {
    if (is_dashed) {
      arrow_char = chars->arrow_left_dashed;
    } else if (is_open) {
      arrow_char = chars->arrow_left_open;
    } else if (is_x) {
      arrow_char = chars->arrow_left_x;
    } else {
      arrow_char = chars->arrow_left;
    }
  }

  if (m->inline_left_note_idx == (size_t)-1 &&
      m->inline_right_note_idx == (size_t)-1) {
    col = 0;
    for (size_t i = 0; i < d->participant_count; i++) {
      size_t pos = layout->positions[i];
      if (print_spaces_safe(output, col, pos) < 0)
        return -1;

      if (left_to_right) {
        if (pos == from_pos) {
          if (fputs(chars->life_branch_left, output) == EOF)
            return -1;

          if (line_span > 0) {
            if (print_repeat(output, line_char, line_span) < 0)
              return -1;
          }
          if (fputs(arrow_char, output) == EOF)
            return -1;
          col = right_pos;
          while (i + 1 < d->participant_count &&
                 layout->positions[i + 1] < right_pos)
            i++;
        } else {
          if (fputs(chars->life_v, output) == EOF)
            return -1;
          col = pos + 1;
        }
      } else {
        if (pos == left_pos) {
          if (fputs(chars->life_v, output) == EOF)
            return -1;
          if (fputs(arrow_char, output) == EOF)
            return -1;
          if (line_span > 0) {
            if (print_repeat(output, line_char, line_span) < 0)
              return -1;
          }
          if (fputs(chars->life_branch_right, output) == EOF)
            return -1;
          col = right_pos + 1;
          while (i + 1 < d->participant_count &&
                 layout->positions[i + 1] <= right_pos)
            i++;
        } else {
          if (fputs(chars->life_v, output) == EOF)
            return -1;
          col = pos + 1;
        }
      }
    }
    if (fputc('\n', output) == EOF)
      return -1;
    return 0;
  }

  size_t left_note_len = 0;
  size_t right_note_len = 0;
  const char *left_note_text =
      left_note ? get_line(left_note->text, 0, &left_note_len) : NULL;
  const char *right_note_text =
      right_note ? get_line(right_note->text, 0, &right_note_len) : NULL;

  if (render_inline_row(output, d, layout, chars, true, left_to_right, from_pos,
                        left_pos, right_pos, line_char, arrow_char, line_span,
                        left_note, left_box_start, left_box_end, chars->box_v,
                        chars->box_v, left_note_text, left_note_len, right_note,
                        right_box_start, right_box_end, chars->box_v,
                        chars->box_v, right_note_text, right_note_len) < 0)
    return -1;

  size_t remaining = 0;
  if (left_note && left_note->line_count > remaining)
    remaining = left_note->line_count;
  if (right_note && right_note->line_count > remaining)
    remaining = right_note->line_count;

  for (size_t ln = 1; ln <= remaining; ln++) {
    const char *left_text = NULL;
    const char *right_text = NULL;
    size_t left_len = 0;
    size_t right_len = 0;
    const char *left_edge = NULL;
    const char *left_right_edge = NULL;
    const char *right_left_edge = NULL;
    const char *right_right_edge = NULL;
    bool draw_left = left_note && ln <= left_note->line_count;
    bool draw_right = right_note && ln <= right_note->line_count;

    if (draw_left) {
      if (ln < left_note->line_count) {
        left_text = get_line(left_note->text, ln, &left_len);
        left_edge = chars->box_v;
        left_right_edge = chars->box_v;
      } else {
        left_edge = chars->box_bl;
        left_right_edge = chars->box_br;
      }
    }

    if (draw_right) {
      if (ln < right_note->line_count) {
        right_text = get_line(right_note->text, ln, &right_len);
        right_left_edge = chars->box_v;
        right_right_edge = chars->box_v;
      } else {
        right_left_edge = chars->box_bl;
        right_right_edge = chars->box_br;
      }
    }

    if (render_inline_row(
            output, d, layout, chars, false, left_to_right, from_pos, left_pos,
            right_pos, line_char, arrow_char, line_span, draw_left,
            left_box_start, left_box_end, left_edge ? left_edge : chars->box_v,
            left_right_edge ? left_right_edge : chars->box_v, left_text,
            left_len, draw_right, right_box_start, right_box_end,
            right_left_edge ? right_left_edge : chars->box_v,
            right_right_edge ? right_right_edge : chars->box_v, right_text,
            right_len) < 0)
      return -1;
  }
  return 0;
}

bool render_diagram(Diagram *d, RenderMode mode, FILE *output) {
  if (!d || d->participant_count == 0)
    return true;

  if (!output)
    output = stdout;

  const RenderChars *chars = (mode == RENDER_UTF8) ? &UTF8_CHARS : &ASCII_CHARS;
  Layout *layout = compute_layout(d);
  if (!layout)
    return false;

  if (render_header_top(output, d, layout, chars) < 0)
    goto render_error;
  if (render_header_name(output, d, layout, chars) < 0)
    goto render_error;
  if (render_header_bottom(output, d, layout, chars) < 0)
    goto render_error;

  if (render_lifeline(output, d, layout, chars) < 0)
    goto render_error;
  if (render_lifeline(output, d, layout, chars) < 0)
    goto render_error;

  for (size_t i = 0; i < d->event_count; i++) {
    DiagramEvent *event = &d->events[i];
    if (event->type == EVENT_MESSAGE) {
      if (render_message(output, d, layout, &d->messages[event->index], chars) <
          0)
        goto render_error;
    } else {
      if (render_note(output, d, layout, &d->notes[event->index], chars) < 0)
        goto render_error;
    }
  }

  for (int i = 0; i < 5; i++) {
    if (render_lifeline(output, d, layout, chars) < 0)
      goto render_error;
  }

  if (fputc('\n', output) == EOF)
    goto render_error;

  layout_free(layout);
  return true;

render_error:
  layout_free(layout);
  return false;
}
