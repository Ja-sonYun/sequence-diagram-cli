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
// #define DEFAULT
#define UTF_DEFAULT
// #define UTF_DOUBLE_LINE
// #define UTF_BOX
// #define UTF_DOTTED
//--------------------------------------------------
#ifdef DEFAULT
#   define PARTICIPANT_VERTICAL_LINE       "|"
#   define PARTICIPANT_HORIZONTAL_LINE     "-"
#   define PARTICIPANT_TOP_LEFT            "+"
#   define PARTICIPANT_TOP_RIGHT           "+"
#   define PARTICIPANT_BOTTOM_LEFT         "+"
#   define PARTICIPANT_BOTTOM_RIGHT        "+"
#   define PARTICIPANT_BOTTOM_CONNECTION   "+"
#   define VERTICAL_LINE                   "|"
#endif
//--------------------------------------------------
#ifdef  UTF_DEFAULT
#   define UTF_SUPPORT
#   define PARTICIPANT_VERTICAL_LINE       "│"
#   define PARTICIPANT_HORIZONTAL_LINE     "─"
#   define PARTICIPANT_TOP_LEFT            "╭"
#   define PARTICIPANT_TOP_RIGHT           "╮"
#   define PARTICIPANT_BOTTOM_LEFT         "╰"
#   define PARTICIPANT_BOTTOM_RIGHT        "╯"
#   define PARTICIPANT_BOTTOM_CONNECTION   "┬"
#   define VERTICAL_LINE                   "│"
#   define ARROW_NORMAL_VERTICAL_LINE      "│"
#   define ARROW_RET_VERTICAL_LINE         "┊"
#   define ARROW_RET_LINE_L                "╴"
#   define ARROW_RET_LINE_R                "╶"
#   define ARROW_NORMAL_R                  "▶"
#   define ARROW_NORMAL_L                  "◀"
#   define ARROW_RET_L                     "≺"
#   define ARROW_RET_R                     "≻"
#   define ARROW_ORIGIN_R                  "├"
#   define ARROW_ORIGIN_L                  "┤"
#   define ARROW_LINE_R                    "╌"
#   define ARROW_LINE_L                    "╌"
#endif
//--------------------------------------------------
#ifdef UTF_DOUBLE_LINE
#   define UTF_SUPPORT
#   define PARTICIPANT_VERTICAL_LINE       "║"
#   define PARTICIPANT_HORIZONTAL_LINE     "═"
#   define PARTICIPANT_TOP_LEFT            "╔"
#   define PARTICIPANT_TOP_RIGHT           "╗"
#   define PARTICIPANT_BOTTOM_LEFT         "╚"
#   define PARTICIPANT_BOTTOM_RIGHT        "╝"
#   define PARTICIPANT_BOTTOM_CONNECTION   "╤"
#   define VERTICAL_LINE                   "│"
#endif
//--------------------------------------------------
#ifdef  UTF_BOX
#   define UTF_SUPPORT
#   define PARTICIPANT_VERTICAL_LINE       "│"
#   define PARTICIPANT_HORIZONTAL_LINE     "─"
#   define PARTICIPANT_TOP_LEFT            "┌"
#   define PARTICIPANT_TOP_RIGHT           "┐"
#   define PARTICIPANT_BOTTOM_LEFT         "└"
#   define PARTICIPANT_BOTTOM_RIGHT        "┘"
#   define PARTICIPANT_BOTTOM_CONNECTION   "┬"
#   define VERTICAL_LINE                   "│"
#endif
//--------------------------------------------------
#ifdef  UTF_DOTTED
#   define UTF_SUPPORT
#   define PARTICIPANT_VERTICAL_LINE       "┊"
#   define PARTICIPANT_HORIZONTAL_LINE     "┄"
#   define PARTICIPANT_TOP_LEFT            "+"
#   define PARTICIPANT_TOP_RIGHT           "+"
#   define PARTICIPANT_BOTTOM_LEFT         "+"
#   define PARTICIPANT_BOTTOM_RIGHT        "+"
#   define PARTICIPANT_BOTTOM_CONNECTION   "+"
#   define VERTICAL_LINE                   "┊"
#endif
//--------------------------------------------------

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

#endif
