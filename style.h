#ifndef STYLE_H
#define STYLE_H

#define PARTICIPANT_TOP_GAP 0
#define PARTICIPANT_BOTTOM_GAP 0
#define PARTICIPANT_VERTICAL_GAP (PARTICIPANT_BOTTOM_GAP+PARTICIPANT_TOP_GAP+2)
//  +-------+ <-+
//  | somth |   |
//  +-------+ <-+- : default = 2
#define PARTICIPANT_LEFT_GAP 1
#define PARTICIPANT_RIGHT_GAP 1
#define PARTICIPANT_HORIZONTAL_GAP (PARTICIPANT_LEFT_GAP+PARTICIPANT_RIGHT_GAP+2)
//  +-------+
//  | somth |
//  +-------+
//  ^-------^---: default = 2
//--------------------------------------------------
//      ***STYLES***
#define DEFAULT 0
#define UTF_DEFAULT 1
//--------------------------------------------------
#define STYLE_S 21
#define UTF_SUPPORT
#   define D_PARTICIPANT_VERTICAL_LINE       "|"
#   define D_PARTICIPANT_HORIZONTAL_LINE     "-"
#   define D_PARTICIPANT_TOP_LEFT            "+"
#   define D_PARTICIPANT_TOP_RIGHT           "+"
#   define D_PARTICIPANT_BOTTOM_LEFT         "+"
#   define D_PARTICIPANT_BOTTOM_RIGHT        "+"
#   define D_PARTICIPANT_BOTTOM_CONNECTION   "+"
#   define D_VERTICAL_LINE                   "|"
#   define D_ARROW_NORMAL_VERTICAL_LINE      "|"
#   define D_ARROW_RET_VERTICAL_LINE         ":"
#   define D_ARROW_RET_LINE_L                "."
#   define D_ARROW_RET_LINE_R                "."
#   define D_ARROW_NORMAL_R                  ">"
#   define D_ARROW_NORMAL_L                  "<"
#   define D_ARROW_RET_L                     "<"
#   define D_ARROW_RET_R                     ">"
#   define D_ARROW_ORIGIN_R                  "+"
#   define D_ARROW_ORIGIN_L                  "+"
#   define D_ARROW_LINE_R                    "-"
#   define D_ARROW_LINE_L                    "-"
//--------------------------------------------------
#   define U_PARTICIPANT_VERTICAL_LINE       "│"
#   define U_PARTICIPANT_HORIZONTAL_LINE     "─"
#   define U_PARTICIPANT_TOP_LEFT            "╭"
#   define U_PARTICIPANT_TOP_RIGHT           "╮"
#   define U_PARTICIPANT_BOTTOM_LEFT         "╰"
#   define U_PARTICIPANT_BOTTOM_RIGHT        "╯"
#   define U_PARTICIPANT_BOTTOM_CONNECTION   "┬"
#   define U_VERTICAL_LINE                   "│"
#   define U_ARROW_NORMAL_VERTICAL_LINE      "│"
#   define U_ARROW_RET_VERTICAL_LINE         "┊"
#   define U_ARROW_RET_LINE_L                "╴"
#   define U_ARROW_RET_LINE_R                "╶"
#   define U_ARROW_NORMAL_R                  "▶"
#   define U_ARROW_NORMAL_L                  "◀"
#   define U_ARROW_RET_L                     "≺"
#   define U_ARROW_RET_R                     "≻"
#   define U_ARROW_ORIGIN_R                  "├"
#   define U_ARROW_ORIGIN_L                  "┤"
#   define U_ARROW_LINE_R                    "╌"
#   define U_ARROW_LINE_L                    "╌"
//--------------------------------------------------
#define PARTICIPANT_VERTICAL_LINE style[ 0 ]
#define PARTICIPANT_HORIZONTAL_LINE style[ 1 ]
#define PARTICIPANT_TOP_LEFT style[ 2 ]
#define PARTICIPANT_TOP_RIGHT style[ 3 ]
#define PARTICIPANT_BOTTOM_LEFT style[ 4 ]
#define PARTICIPANT_BOTTOM_RIGHT style[ 5 ]
#define PARTICIPANT_BOTTOM_CONNECTION style[ 6 ]
#define VERTICAL_LINE style[ 7 ]
#define ARROW_NORMAL_VERTICAL_LINE style[ 8 ]
#define ARROW_RET_VERTICAL_LINE style[ 9 ]
#define ARROW_RET_LINE_L style[ 10 ]
#define ARROW_RET_LINE_R style[ 11 ]
#define ARROW_NORMAL_R style[ 12 ]
#define ARROW_NORMAL_L style[ 13 ]
#define ARROW_RET_L style[ 14 ]
#define ARROW_RET_R style[ 15 ]
#define ARROW_ORIGIN_R style[ 16 ]
#define ARROW_ORIGIN_L style[ 17 ]
#define ARROW_LINE_R style[ 18 ]
#define ARROW_LINE_L style[ 19 ]

#ifdef UTF_SUPPORT
static inline char* C(char* ch)
{
    return ch;
}
#else
static inline char C(char* ch)
{
    return *ch;
}
#endif

char* style[STYLE_S];
void set_style(int style);
#endif
