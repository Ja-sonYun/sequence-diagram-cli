#include "style.h"

char *style[STYLE_S];

void set_style(int stylet)
{
    if (stylet)
    {
        PARTICIPANT_VERTICAL_LINE = D_PARTICIPANT_VERTICAL_LINE;
        PARTICIPANT_HORIZONTAL_LINE = D_PARTICIPANT_HORIZONTAL_LINE;
        PARTICIPANT_TOP_LEFT = D_PARTICIPANT_TOP_LEFT;
        PARTICIPANT_TOP_RIGHT = D_PARTICIPANT_TOP_RIGHT;
        PARTICIPANT_BOTTOM_LEFT = D_PARTICIPANT_BOTTOM_LEFT;
        PARTICIPANT_BOTTOM_RIGHT = D_PARTICIPANT_BOTTOM_RIGHT;
        PARTICIPANT_BOTTOM_CONNECTION = D_PARTICIPANT_BOTTOM_CONNECTION;
        VERTICAL_LINE = D_VERTICAL_LINE;
        ARROW_NORMAL_VERTICAL_LINE = D_ARROW_NORMAL_VERTICAL_LINE;
        ARROW_RET_VERTICAL_LINE = D_ARROW_RET_VERTICAL_LINE;
        ARROW_RET_LINE_L = D_ARROW_RET_LINE_L;
        ARROW_RET_LINE_R = D_ARROW_RET_LINE_R;
        ARROW_NORMAL_R = D_ARROW_NORMAL_R;
        ARROW_NORMAL_L = D_ARROW_NORMAL_L;
        ARROW_RET_L = D_ARROW_RET_L;
        ARROW_RET_R = D_ARROW_RET_R;
        ARROW_ORIGIN_R = D_ARROW_ORIGIN_R;
        ARROW_ORIGIN_L = D_ARROW_ORIGIN_L;
        ARROW_LINE_R = D_ARROW_LINE_R;
        ARROW_LINE_L = D_ARROW_LINE_L;
    }
    else
    {
        PARTICIPANT_VERTICAL_LINE = U_PARTICIPANT_VERTICAL_LINE;
        PARTICIPANT_HORIZONTAL_LINE = U_PARTICIPANT_HORIZONTAL_LINE;
        PARTICIPANT_TOP_LEFT = U_PARTICIPANT_TOP_LEFT;
        PARTICIPANT_TOP_RIGHT = U_PARTICIPANT_TOP_RIGHT;
        PARTICIPANT_BOTTOM_LEFT = U_PARTICIPANT_BOTTOM_LEFT;
        PARTICIPANT_BOTTOM_RIGHT = U_PARTICIPANT_BOTTOM_RIGHT;
        PARTICIPANT_BOTTOM_CONNECTION = U_PARTICIPANT_BOTTOM_CONNECTION;
        VERTICAL_LINE = U_VERTICAL_LINE;
        ARROW_NORMAL_VERTICAL_LINE = U_ARROW_NORMAL_VERTICAL_LINE;
        ARROW_RET_VERTICAL_LINE = U_ARROW_RET_VERTICAL_LINE;
        ARROW_RET_LINE_L = U_ARROW_RET_LINE_L;
        ARROW_RET_LINE_R = U_ARROW_RET_LINE_R;
        ARROW_NORMAL_R = U_ARROW_NORMAL_R;
        ARROW_NORMAL_L = U_ARROW_NORMAL_L;
        ARROW_RET_L = U_ARROW_RET_L;
        ARROW_RET_R = U_ARROW_RET_R;
        ARROW_ORIGIN_R = U_ARROW_ORIGIN_R;
        ARROW_ORIGIN_L = U_ARROW_ORIGIN_L;
        ARROW_LINE_R = U_ARROW_LINE_R;
        ARROW_LINE_L = U_ARROW_LINE_L;
    }
}