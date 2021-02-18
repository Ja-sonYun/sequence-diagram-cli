#ifndef COMPILER_H
#define COMPILER_H
#endif

#include "parser.h"
#include "style.h"
#include <math.h>

extern char *prefix;
extern char *suffix;

#define GET_LINE_M(from, to) \
    (to - from) / 2 + from

typedef struct _coordinates
{
    int x;
    int y;
} Pos;

#define POS_INIT { .x = 0, .y = 0 }

struct arrow_def_coor
{
    int top_padding;
    Pos size;
    Pos from;
    Pos to;
};

struct group
{
    Pos from;
    Pos to;
    char **cases;
    int cases_num;
};

struct participant_render
{
    int left_padding;
    int right_padding;
    int middle;
    Pos from;
    Pos word_size;
    Pos to;
};

struct lifeline
{
    Pos from;
    Pos to;
};

struct seperater
{
    int x;
};

typedef struct _area
{
    char *title;
    struct _header
    {
#ifdef UTF_SUPPORT
        char ***buffer;
#else
        char **buffer;
#endif
        Pos pos;
        struct participant_render *participants;
    } header;
    struct _body
    {
#ifdef UTF_SUPPORT
        char ***buffer;
#else
        char **buffer;
#endif
        Pos pos;
        int *e_s;
        int l_p;
        struct lifeline *lifelines;
        struct group *groups;
        struct arrow_def_coor *arrow_defs;
    } body;
    Pos pos;
} Area;

void render();
void render_test();

// debug

static inline void _debug_header_(Area *area)
{
    for (int i = 0; i < participants.members_num; i++)
    {
        printf("participant '%s'\n", participants.members[i]->name->string);
        printf("from -> x: %d, y: %d\n", area->header.participants[i].from.x, area->header.participants[i].from.y);
        printf("to   -> x: %d, y: %d\n", area->header.participants[i].to.x, area->header.participants[i].to.y);
        printf("size -> x: %d, y: %d\n", area->header.participants[i].word_size.x, area->header.participants[i].word_size.y);
        printf("============================\n");
    }
    printf("xpos -> x: %d, y: %d\n", area->header.pos.x, area->header.pos.y);
}

