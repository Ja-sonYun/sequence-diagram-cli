#include "renderer.h"
#include "../model/text_utils.h"
#include "../model/types.h"
#include "canvas.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *box_tl;
  const char *box_tr;
  const char *box_bl;
  const char *box_br;
  const char *box_h;
  const char *box_v;
  const char *box_bt;
  const char *note_tl;
  const char *note_tr;
  const char *note_bl;
  const char *note_br;
  const char *note_th;
  const char *life_v;
  const char *loop_v;
  const char *life_branch_left;
  const char *life_branch_right;
  const char *arrow_right;
  const char *arrow_left;
  const char *arrow_right_open;
  const char *arrow_left_open;
  const char *arrow_right_dashed;
  const char *arrow_left_dashed;
  const char *arrow_right_x;
  const char *arrow_left_x;
  const char *line_solid;
  const char *line_dashed;
} RenderChars;

static const RenderChars UTF8_CHARS = {
    .box_tl = "╭",
    .box_tr = "╮",
    .box_bl = "╰",
    .box_br = "╯",
    .box_h = "─",
    .box_v = "│",
    .box_bt = "┬",
    .note_tl = "╒",
    .note_tr = "╕",
    .note_bl = "└",
    .note_br = "┘",
    .note_th = "═",
    .life_v = "┆",
    .loop_v = "│",
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
    .note_tl = "+",
    .note_tr = "+",
    .note_bl = "+",
    .note_br = "+",
    .note_th = "=",
    .life_v = "|",
    .loop_v = "|",
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
    .line_dashed = ".",
};

enum {
  BOX_PADDING = 2,
  MIN_BOX_WIDTH = 3,
  BASE_GAP = 1,
  MSG_PADDING = 2,
  NOTE_BOX_EXTRA = 4,
  NOTE_BORDER_HEIGHT = 2,
  LIFELINE_GAP = 0,
  SELF_MSG_TEXT_OFFSET = 2,
  HEADER_SPACING = 1,
  EVENT_SPACING = 1,
  FOOTER_SPACING = 2,
};

typedef struct {
  size_t *positions;
  size_t *widths;
  size_t *gaps;
  size_t left_margin;
  size_t count;
  size_t total_width;

  size_t max_name_lines;
  size_t header_height;
  size_t *event_y;
  size_t *event_heights;
  size_t total_height;
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

static bool arrow_is_dashed(ArrowType arrow) {
  return arrow == ARROW_DASHED || arrow == ARROW_DASHED_LEFT;
}

static bool arrow_is_open(ArrowType arrow) {
  return arrow == ARROW_OPEN || arrow == ARROW_OPEN_LEFT;
}

static bool arrow_is_x(ArrowType arrow) {
  return arrow == ARROW_X || arrow == ARROW_X_LEFT;
}

static const char *select_arrow_char(const RenderChars *chars, ArrowType arrow,
                                     bool left_to_right) {
  if (left_to_right) {
    if (arrow_is_dashed(arrow))
      return chars->arrow_right_dashed;
    if (arrow_is_open(arrow))
      return chars->arrow_right_open;
    if (arrow_is_x(arrow))
      return chars->arrow_right_x;
    return chars->arrow_right;
  }

  if (arrow_is_dashed(arrow))
    return chars->arrow_left_dashed;
  if (arrow_is_open(arrow))
    return chars->arrow_left_open;
  if (arrow_is_x(arrow))
    return chars->arrow_left_x;
  return chars->arrow_left;
}

static size_t calc_span(Layout *layout, size_t left, size_t right) {
  size_t span = layout->widths[left] / 2 + layout->widths[right] / 2;
  span += layout->gaps[left];
  for (size_t j = left + 1; j < right; j++) {
    span += layout->widths[j] + layout->gaps[j];
  }
  return span;
}

static void expand_gaps(Layout *layout, size_t left, size_t right,
                        size_t required) {
  size_t current = calc_span(layout, left, right);
  if (required <= current)
    return;

  size_t extra = required - current;
  size_t gap_count = right - left;

  if (gap_count > 0) {
    size_t per_gap = extra / gap_count;
    size_t remainder = extra % gap_count;
    for (size_t j = left; j < right; j++) {
      layout->gaps[j] += per_gap + (j - left < remainder ? 1 : 0);
    }
  } else {
    layout->gaps[left] += extra;
  }
}

static void layout_self_message(Layout *layout, Message *m, size_t count) {
  size_t loop_width = max_line_width(m->text) + NOTE_BOX_EXTRA;
  size_t required = loop_width + LIFELINE_GAP;
  size_t idx = m->from_idx;

  if (idx + 1 < count) {
    size_t current = layout->widths[idx] / 2 + layout->widths[idx + 1] / 2 +
                     layout->gaps[idx];
    if (required > current)
      layout->gaps[idx] += required - current;
  } else {
    size_t current = layout->widths[idx] / 2 + layout->gaps[idx];
    if (required > current)
      layout->gaps[idx] += required - current;
  }
}

static size_t calc_message_height(Diagram *d, Message *m) {
  if (m->is_self) {
    return count_lines(m->text) + 3 + EVENT_SPACING;
  }

  size_t height = count_lines(m->text) + 1;

  size_t left_lines = 0, right_lines = 0;
  if (m->inline_left_note_idx != INVALID_INDEX)
    left_lines = d->notes[m->inline_left_note_idx].line_count;
  if (m->inline_right_note_idx != INVALID_INDEX)
    right_lines = d->notes[m->inline_right_note_idx].line_count;

  size_t max_note = left_lines > right_lines ? left_lines : right_lines;
  if (max_note > 0)
    height += max_note;

  return height + EVENT_SPACING;
}

static size_t calc_note_height(Note *note) {
  return note->line_count + 2 + EVENT_SPACING;
}

static void layout_init_widths_and_gaps(Layout *layout, Diagram *d) {
  for (size_t i = 0; i < d->participant_count; i++) {
    const Participant *p = &d->participants[i];
    layout->widths[i] = p->max_line_width + BOX_PADDING * 2;
    if (layout->widths[i] < MIN_BOX_WIDTH)
      layout->widths[i] = MIN_BOX_WIDTH;
    layout->gaps[i] = (i + 1 < d->participant_count) ? BASE_GAP : 0;
  }
}

static void layout_expand_for_messages(Layout *layout, Diagram *d) {
  for (size_t i = 0; i < d->message_count; i++) {
    Message *m = &d->messages[i];
    if (m->from_idx == m->to_idx) {
      layout_self_message(layout, m, d->participant_count);
      continue;
    }

    size_t left = m->from_idx < m->to_idx ? m->from_idx : m->to_idx;
    size_t right = m->from_idx < m->to_idx ? m->to_idx : m->from_idx;
    size_t msg_width = max_line_width(m->text) + MSG_PADDING;
    expand_gaps(layout, left, right, msg_width);

    if (m->inline_left_note_idx != INVALID_INDEX) {
      Note *note = &d->notes[m->inline_left_note_idx];
      size_t box_width = note->max_line_width + NOTE_BOX_EXTRA;
      size_t needed = box_width + LIFELINE_GAP;
      if (left > 0) {
        size_t avail = layout->widths[left - 1] / 2 + layout->gaps[left - 1] +
                       layout->widths[left] / 2;
        if (needed > avail)
          layout->gaps[left - 1] += needed - avail;
      } else {
        size_t avail = layout->left_margin + layout->widths[left] / 2;
        if (needed > avail)
          layout->left_margin += needed - avail;
      }
    }
    if (m->inline_right_note_idx != INVALID_INDEX) {
      Note *note = &d->notes[m->inline_right_note_idx];
      size_t box_width = note->max_line_width + NOTE_BOX_EXTRA;
      size_t needed = box_width + LIFELINE_GAP;
      size_t avail = layout->gaps[right] + layout->widths[right] / 2;
      if (needed > avail)
        layout->gaps[right] += needed - avail;
    }
  }
}

static void layout_expand_for_notes(Layout *layout, Diagram *d) {
  for (size_t i = 0; i < d->note_count; i++) {
    Note *note = &d->notes[i];
    size_t box_width = note->max_line_width + NOTE_BOX_EXTRA;
    size_t left = note->from_idx;
    size_t right = note->to_idx;
    size_t needed = box_width + LIFELINE_GAP;

    if (note->position == NOTE_LEFT) {
      if (left > 0) {
        size_t avail = layout->widths[left - 1] / 2 + layout->gaps[left - 1] +
                       layout->widths[left] / 2;
        if (needed > avail)
          layout->gaps[left - 1] += needed - avail;
      } else {
        size_t avail = layout->left_margin + layout->widths[left] / 2;
        if (needed > avail)
          layout->left_margin += needed - avail;
      }
    } else if (note->position == NOTE_RIGHT) {
      if (right + 1 < d->participant_count) {
        size_t avail = layout->widths[right] / 2 + layout->gaps[right] +
                       layout->widths[right + 1] / 2;
        if (needed > avail)
          layout->gaps[right] += needed - avail;
      } else {
        layout->gaps[right] += needed;
      }
    } else {
      expand_gaps(layout, left, right, needed);
    }
  }
}

static void layout_finalize_positions(Layout *layout) {
  size_t pos = layout->left_margin;
  for (size_t i = 0; i < layout->count; i++) {
    layout->positions[i] = pos + layout->widths[i] / 2;
    pos += layout->widths[i] + layout->gaps[i];
  }
  layout->total_width = pos;
}

static void layout_compute_heights(Layout *layout, Diagram *d) {
  layout->max_name_lines = 1;
  for (size_t i = 0; i < d->participant_count; i++) {
    if (d->participants[i].line_count > layout->max_name_lines)
      layout->max_name_lines = d->participants[i].line_count;
  }
  layout->header_height = 2 + layout->max_name_lines + HEADER_SPACING;

  size_t y = layout->header_height;
  for (size_t i = 0; i < d->event_count; i++) {
    layout->event_y[i] = y;
    DiagramEvent *event = &d->events[i];

    if (event->type == EVENT_MESSAGE) {
      layout->event_heights[i] =
          calc_message_height(d, &d->messages[event->index]);
    } else {
      layout->event_heights[i] = calc_note_height(&d->notes[event->index]);
    }
    y += layout->event_heights[i];
  }

  layout->total_height = y + FOOTER_SPACING;
}

static Layout *compute_layout(Diagram *d) {
  Layout *layout = malloc(sizeof(Layout));
  if (!layout)
    return NULL;

  layout->count = d->participant_count;
  layout->left_margin = 0;
  layout->positions = calloc(layout->count, sizeof(size_t));
  layout->widths = calloc(layout->count, sizeof(size_t));
  layout->gaps = calloc(layout->count, sizeof(size_t));
  layout->event_y = calloc(d->event_count, sizeof(size_t));
  layout->event_heights = calloc(d->event_count, sizeof(size_t));

  if (!layout->positions || !layout->widths || !layout->gaps ||
      !layout->event_y || !layout->event_heights) {
    free(layout->positions);
    free(layout->widths);
    free(layout->gaps);
    free(layout->event_y);
    free(layout->event_heights);
    free(layout);
    return NULL;
  }

  layout_init_widths_and_gaps(layout, d);
  layout_expand_for_messages(layout, d);
  layout_expand_for_notes(layout, d);
  layout_finalize_positions(layout);
  layout_compute_heights(layout, d);

  return layout;
}

static void layout_free(Layout *layout) {
  if (!layout)
    return;
  free(layout->positions);
  free(layout->widths);
  free(layout->gaps);
  free(layout->event_y);
  free(layout->event_heights);
  free(layout);
}

static void draw_lifelines(Canvas *c, Diagram *d, Layout *layout,
                           const RenderChars *chars) {
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t x = layout->positions[i];
    canvas_vline(c, x, 0, layout->total_height - 1, chars->life_v);
  }
}

static void draw_header(Canvas *c, Diagram *d, Layout *layout,
                        const RenderChars *chars) {
  for (size_t i = 0; i < d->participant_count; i++) {
    size_t pos = layout->positions[i];
    size_t width = layout->widths[i];
    size_t box_start = pos - width / 2;
    size_t box_end = box_start + width;

    canvas_put(c, box_start, 0, chars->box_tl);
    canvas_hline(c, box_start + 1, box_end - 2, 0, chars->box_h);
    canvas_put(c, box_end - 1, 0, chars->box_tr);

    const Participant *p = &d->participants[i];
    for (size_t ln = 0; ln < layout->max_name_lines; ln++) {
      size_t row = 1 + ln;
      canvas_put(c, box_start, row, chars->box_v);
      canvas_put(c, box_end - 1, row, chars->box_v);

      size_t text_len = 0;
      const char *text = get_line(p->name, ln, &text_len);
      if (text) {
        size_t text_width = utf8_display_width_n(text, text_len);
        size_t inner = width - 2;
        size_t pad = (inner > text_width) ? (inner - text_width) / 2 : 0;
        canvas_puts_n(c, box_start + 1 + pad, row, text, text_len);
      }
    }

    size_t bottom = 1 + layout->max_name_lines;
    size_t half = width / 2;
    canvas_put(c, box_start, bottom, chars->box_bl);
    canvas_hline(c, box_start + 1, box_start + half - 1, bottom, chars->box_h);
    canvas_put(c, pos, bottom, chars->box_bt);
    canvas_hline(c, pos + 1, box_end - 2, bottom, chars->box_h);
    canvas_put(c, box_end - 1, bottom, chars->box_br);
  }
}

static size_t calc_note_box_start(Layout *layout, Note *note,
                                  size_t box_width) {
  size_t left_pos = layout->positions[note->from_idx];
  size_t right_pos = layout->positions[note->to_idx];

  if (note->position == NOTE_LEFT) {
    size_t offset = box_width + LIFELINE_GAP;
    return left_pos > offset ? left_pos - offset : 0;
  }
  if (note->position == NOTE_RIGHT) {
    return right_pos + 1 + LIFELINE_GAP;
  }

  size_t span = right_pos - left_pos;
  if (span >= box_width)
    return left_pos + (span - box_width) / 2;
  return left_pos > (box_width - span) / 2 ? left_pos - (box_width - span) / 2
                                           : 0;
}

static void draw_note_box(Canvas *c, Layout *layout, Note *note, size_t y,
                          const RenderChars *chars) {
  size_t box_width = note->max_line_width + NOTE_BOX_EXTRA;
  size_t box_start = calc_note_box_start(layout, note, box_width);
  size_t box_end = box_start + box_width;
  size_t inner = box_width - 2;

  canvas_put(c, box_start, y, chars->note_tl);
  canvas_hline(c, box_start + 1, box_end - 2, y, chars->note_th);
  canvas_put(c, box_end - 1, y, chars->note_tr);

  for (size_t ln = 0; ln < note->line_count; ln++) {
    size_t row = y + 1 + ln;
    canvas_put(c, box_start, row, chars->box_v);
    canvas_put(c, box_end - 1, row, chars->box_v);

    size_t text_len = 0;
    const char *text = get_line(note->text, ln, &text_len);
    if (text) {
      size_t tw = utf8_display_width_n(text, text_len);
      size_t pad = (inner > tw) ? (inner - tw) / 2 : 0;
      canvas_puts_n(c, box_start + 1 + pad, row, text, text_len);
    }
  }

  size_t bottom = y + 1 + note->line_count;
  canvas_put(c, box_start, bottom, chars->note_bl);
  canvas_hline(c, box_start + 1, box_end - 2, bottom, chars->box_h);
  canvas_put(c, box_end - 1, bottom, chars->note_br);
}

static void draw_self_message(Canvas *c, Layout *layout, Message *m, size_t y,
                              const RenderChars *chars) {
  size_t pos = layout->positions[m->from_idx];
  size_t loop_width = max_line_width(m->text) + NOTE_BOX_EXTRA;
  size_t num_lines = count_lines(m->text);

  bool is_dashed = arrow_is_dashed(m->arrow);
  bool is_x = arrow_is_x(m->arrow);
  const char *line_char = is_dashed ? chars->line_dashed : chars->line_solid;
  const char *arrow_char =
      is_dashed ? chars->arrow_left_dashed
                : (is_x ? chars->arrow_left_x : chars->arrow_left);

  size_t cur_y = y;

  for (size_t ln = 0; ln < num_lines; ln++) {
    size_t text_len;
    const char *text = get_line(m->text, ln, &text_len);
    canvas_puts_n(c, pos + SELF_MSG_TEXT_OFFSET, cur_y, text, text_len);
    cur_y++;
  }

  size_t loop_end = pos + 1 + loop_width;
  canvas_put(c, pos, cur_y, chars->life_branch_left);
  canvas_hline(c, pos + 1, loop_end - 1, cur_y, line_char);
  canvas_put(c, loop_end, cur_y, chars->box_tr);
  cur_y++;

  canvas_put(c, loop_end, cur_y, chars->loop_v);
  cur_y++;

  canvas_put(c, pos, cur_y, arrow_char);
  canvas_hline(c, pos + 1, loop_end - 1, cur_y, line_char);
  canvas_put(c, loop_end, cur_y, chars->box_br);
}

static void draw_inline_note(Canvas *c, Note *note, size_t anchor_pos,
                             bool is_left, size_t arrow_y,
                             const RenderChars *chars) {
  size_t box_width = note->max_line_width + NOTE_BOX_EXTRA;
  size_t box_start = is_left ? (anchor_pos > box_width + LIFELINE_GAP
                                    ? anchor_pos - box_width - LIFELINE_GAP
                                    : 0)
                             : anchor_pos + 1 + LIFELINE_GAP;
  size_t box_end = box_start + box_width;
  size_t inner = box_width - 2;

  size_t top_y = arrow_y - 1;
  canvas_put(c, box_start, top_y, chars->note_tl);
  canvas_hline(c, box_start + 1, box_end - 2, top_y, chars->note_th);
  canvas_put(c, box_end - 1, top_y, chars->note_tr);

  canvas_put(c, box_start, arrow_y, chars->box_v);
  canvas_put(c, box_end - 1, arrow_y, chars->box_v);
  size_t text_len = 0;
  const char *text = get_line(note->text, 0, &text_len);
  if (text) {
    size_t tw = utf8_display_width_n(text, text_len);
    size_t pad = (inner > tw) ? (inner - tw) / 2 : 0;
    canvas_puts_n(c, box_start + 1 + pad, arrow_y, text, text_len);
  }

  for (size_t ln = 1; ln < note->line_count; ln++) {
    size_t row = arrow_y + ln;
    canvas_put(c, box_start, row, chars->box_v);
    canvas_put(c, box_end - 1, row, chars->box_v);

    text = get_line(note->text, ln, &text_len);
    if (text) {
      size_t tw = utf8_display_width_n(text, text_len);
      size_t pad = (inner > tw) ? (inner - tw) / 2 : 0;
      canvas_puts_n(c, box_start + 1 + pad, row, text, text_len);
    }
  }

  size_t bottom = arrow_y + note->line_count;
  canvas_put(c, box_start, bottom, chars->note_bl);
  canvas_hline(c, box_start + 1, box_end - 2, bottom, chars->box_h);
  canvas_put(c, box_end - 1, bottom, chars->note_br);
}

static void draw_message(Canvas *c, Diagram *d, Layout *layout, Message *m,
                         size_t y, const RenderChars *chars) {
  if (m->is_self) {
    draw_self_message(c, layout, m, y, chars);
    return;
  }

  size_t from_pos = layout->positions[m->from_idx];
  size_t to_pos = layout->positions[m->to_idx];
  bool left_to_right = from_pos < to_pos;
  size_t left_pos = left_to_right ? from_pos : to_pos;
  size_t right_pos = left_to_right ? to_pos : from_pos;
  size_t line_width = right_pos - left_pos - 1;

  bool is_dashed = arrow_is_dashed(m->arrow);
  const char *line_char = is_dashed ? chars->line_dashed : chars->line_solid;
  const char *arrow_char = select_arrow_char(chars, m->arrow, left_to_right);

  size_t num_lines = count_lines(m->text);
  size_t cur_y = y;

  for (size_t ln = 0; ln < num_lines; ln++) {
    size_t text_len;
    const char *text = get_line(m->text, ln, &text_len);
    size_t tw = utf8_display_width_n(text, text_len);
    size_t pad = (line_width > tw) ? (line_width - tw) / 2 : 0;
    canvas_puts_n(c, left_pos + 1 + pad, cur_y, text, text_len);
    cur_y++;
  }

  size_t arrow_y = cur_y;
  size_t line_span = line_width > 1 ? line_width - 1 : 0;

  if (left_to_right) {
    canvas_put(c, from_pos, arrow_y, chars->life_branch_left);
    if (line_span > 0)
      canvas_hline(c, from_pos + 1, from_pos + line_span, arrow_y, line_char);
    canvas_put(c, from_pos + line_span + 1, arrow_y, arrow_char);
  } else {
    canvas_put(c, to_pos + 1, arrow_y, arrow_char);
    if (line_span > 0)
      canvas_hline(c, to_pos + 2, to_pos + 1 + line_span, arrow_y, line_char);
    canvas_put(c, from_pos, arrow_y, chars->life_branch_right);
  }

  if (m->inline_left_note_idx != INVALID_INDEX) {
    draw_inline_note(c, &d->notes[m->inline_left_note_idx], left_pos, true,
                     arrow_y, chars);
  }
  if (m->inline_right_note_idx != INVALID_INDEX) {
    draw_inline_note(c, &d->notes[m->inline_right_note_idx], right_pos, false,
                     arrow_y, chars);
  }
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

  Canvas *canvas = canvas_new(layout->total_width, layout->total_height);
  if (!canvas) {
    layout_free(layout);
    return false;
  }

  draw_lifelines(canvas, d, layout, chars);
  draw_header(canvas, d, layout, chars);

  for (size_t i = 0; i < d->event_count; i++) {
    DiagramEvent *event = &d->events[i];
    size_t y = layout->event_y[i];

    if (event->type == EVENT_MESSAGE) {
      draw_message(canvas, d, layout, &d->messages[event->index], y, chars);
    } else {
      draw_note_box(canvas, layout, &d->notes[event->index], y, chars);
    }
  }

  int result = canvas_print(canvas, output);

  canvas_free(canvas);
  layout_free(layout);

  return result == 0;
}
