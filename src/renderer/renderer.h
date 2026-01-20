#ifndef SEQDIA_RENDERER_H
#define SEQDIA_RENDERER_H

#include "../model/types.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum { RENDER_ASCII, RENDER_UTF8 } RenderMode;

bool render_diagram(Diagram *d, RenderMode mode, FILE *output);

#endif
