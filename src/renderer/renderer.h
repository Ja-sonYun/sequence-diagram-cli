#ifndef SEQDIA_RENDERER_H
#define SEQDIA_RENDERER_H

#include "../model/text_utils.h"
#include "../model/types.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum { RENDER_ASCII, RENDER_UTF8 } RenderMode;

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

bool render_diagram(Diagram *d, RenderMode mode, FILE *output);

#endif
