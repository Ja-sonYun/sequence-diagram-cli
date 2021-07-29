#include "renderer.h"

Area init()
{
    struct participant_render* participants_r = (struct participant_render*)malloc(sizeof(struct participant_render) * participants.members_num);
    for (int i = 0; i < participants.members_num; i++)
    {
        participants_r[i].right_padding = 0;
        participants_r[i].left_padding = 0;
    }
    return (Area) {
        .header = {
            .pos = POS_INIT,
            .participants = participants_r,
        },
        .body = {
            .pos = POS_INIT,
            .l_p = 0,
        },
        .pos = POS_INIT,
    };
}

Pos get_word_size(char *word, int length)
{
    Pos w_s = { .x = 0, .y = 1 };
    int wxs = 0;
    int g_c = 0;
    int g_c_y = 0;
    for (int i = 0; i < length; i++) // 3, 4, 3
    {
        if (word[i] == '\n')
        {
            if (w_s.x < wxs) // 3>0 |
                w_s.x = wxs;
            if (g_c_y < g_c)
                g_c_y = g_c;
            g_c = 0;
            wxs = 0;
            w_s.y++;
        }
        else
        {
#if GLOBAL_WS == 3
            if (word[i] >> 7 && word[i+1] >> 7 && word[i+2])
            {
                g_c++;
                i += 2;
            }
#elif GLOBAL_WS == 2
            if (word[i] >> 7 && word[i+1] >> 7)
            {
                g_c++;
                i += 1;
            }
#endif
            else
            {
                wxs++;
            }
        }
    }
    if (w_s.x == 0 || w_s.x < wxs)
        w_s.x = wxs;
    if (g_c_y == 0 || g_c_y < g_c)
        g_c_y = g_c;
    w_s.x = w_s.x + g_c_y * 2;
    return w_s;
}

void cal_coordinates_of_participants(Area *area)
{
    int i, last_y = 0;
    for (i = 0; i < participants.members_num; i++)
    {
        area->header.participants[i].word_size = get_word_size(participants.members[i]->name->string, participants.members[i]->name->length);
        if (last_y < area->header.participants[i].word_size.y)
            last_y = area->header.participants[i].word_size.y;

        if (i == 0)
            area->header.participants[i].from.x = area->header.pos.x + area->header.participants[i].left_padding;
        else
            area->header.participants[i].from.x = area->header.pos.x + area->header.participants[i].left_padding + area->header.participants[i-1].right_padding;
        area->header.participants[i].from.y = 0;

        area->header.participants[i].to.x = area->header.participants[i].word_size.x + PARTICIPANT_HORIZONTAL_GAP + area->header.participants[i].from.x;
        area->header.participants[i].to.y = area->header.participants[i].word_size.y + PARTICIPANT_VERTICAL_GAP + area->header.participants[i].from.y;

        area->header.pos = area->header.participants[i].to;
        area->header.participants[i].middle = GET_LINE_M(area->header.participants[i].from.x, area->header.participants[i].to.x);
    }
    if (last_y > 1)
    {
        last_y = last_y + PARTICIPANT_VERTICAL_GAP;
        for (i = 0; i < participants.members_num; i++)
        {
            area->header.participants[i].to.y = last_y;
            area->header.participants[i].from.y = area->header.participants[i].to.y - area->header.participants[i].word_size.y - PARTICIPANT_VERTICAL_GAP;
        }
        area->header.pos.y = last_y;
    }
}

void recal_coordinates_of_participants(Area *area)
{   // padding, for example
    int i, y, x;
    area->header.participants[0].from.x = 0;
    int pos_x_b = area->header.pos.x;
    area->header.pos.x = 0;
    for (i = 0; i < participants.members_num; i++)
    {
        if (i == 0)
            area->header.participants[i].from.x = area->header.pos.x;
        else
            area->header.participants[i].from.x = area->header.pos.x + area->header.participants[i-1].right_padding;
        area->header.participants[i].to.x = area->header.participants[i].word_size.x + PARTICIPANT_HORIZONTAL_GAP + area->header.participants[i].from.x;
        area->header.pos.x = area->header.participants[i].to.x;
    }
    area->header.pos.x = area->header.pos.x + area->header.participants[participants.members_num-1].right_padding + 2;
    area->body.pos.x = area->header.pos.x;

    bool do_realloc = area->header.pos.x != pos_x_b;
    for (y = 0; y < area->body.pos.y; y++)
    {
        if (do_realloc)
        {
#ifdef UTF_SUPPORT
            area->body.buffer[y] = (char**)realloc_s(area->body.buffer[y], area->body.pos.x * sizeof(char*));
#else
            area->body.buffer[y] = (char*)realloc_s(area->body.buffer[y], area->body.pos.x * sizeof(char));
#endif
        }
        for (x = 0; x < area->body.pos.x; x++)
        {
#ifdef UTF_SUPPORT
            area->body.buffer[y][x] = " ";
#else
            memset(area->body.buffer[y], ' ', area->body.pos.x);
            area->body.buffer[y][area->body.pos.x] = '\0';
#endif
        }
        for (i = 0; i < participants.members_num; i++)
            area->body.buffer[y][GET_LINE_M(area->header.participants[i].from.x, area->header.participants[i].to.x)] = C(VERTICAL_LINE);
    }

}

void add_participants_to_buffer(Area* area)
{
    // TODO: free when this called again
    int y, x, m, t;
#ifdef UTF_SUPPORT
    area->header.buffer = (char***)malloc_s(area->header.pos.y * sizeof(char**));
#else
    area->header.buffer = (char**)malloc_s(area->header.pos.y * sizeof(char*));
#endif
    for (y = 0; y < area->header.pos.y; y++)
    {
#ifdef UTF_SUPPORT
        area->header.buffer[y] = (char**)malloc_s(area->header.pos.x * sizeof(char*));
        for (x = 0; x < area->header.pos.x; x++)
            area->header.buffer[y][x] = " ";
#else
        area->header.buffer[y] = (char*)malloc_s(area->header.pos.x * sizeof(char));
        memset(area->header.buffer[y], ' ', area->header.pos.x);
        area->header.buffer[y][area->header.pos.x] = '\0';
#endif
    }
    // push border to buffer
    for (m = 0; m < participants.members_num; m++)
    {
        for (y = area->header.participants[m].from.y; y < area->header.participants[m].to.y; y++)
        {
            for (x = area->header.participants[m].from.x; x < area->header.participants[m].to.x; x++)
            {
                if (y == area->header.participants[m].from.y) // TOP
                {
                    if (x == area->header.participants[m].from.x)
                        area->header.buffer[y][x] = C(PARTICIPANT_TOP_LEFT); // TOP - LEFT
                    else if (x == area->header.participants[m].to.x - 1)
                        area->header.buffer[y][x] = C(PARTICIPANT_TOP_RIGHT); // TOP - RIGHT
                    else
                        area->header.buffer[y][x] = C(PARTICIPANT_HORIZONTAL_LINE); // TOP - BORDER
                }
                else if (y == area->header.participants[m].to.y - 1) // BOTTOM
                {
                    area->header.participants[m].middle = GET_LINE_M(area->header.participants[m].from.x, area->header.participants[m].to.x);
                    if (area->header.participants[m].middle == x)
                        area->header.buffer[y][x] = C(PARTICIPANT_BOTTOM_CONNECTION); // BOTTOM - LEFT
                    else if (x == area->header.participants[m].from.x)
                        area->header.buffer[y][x] = C(PARTICIPANT_BOTTOM_LEFT); // BOTTOM - LEFT
                    else if (x == area->header.participants[m].to.x - 1)
                        area->header.buffer[y][x] = C(PARTICIPANT_BOTTOM_RIGHT); // BOTTOM - RIGHT
                    else
                        area->header.buffer[y][x] = C(PARTICIPANT_HORIZONTAL_LINE); // BOTTOM - BORDER
                }
                else if (x == area->header.participants[m].from.x ||
                         x == area->header.participants[m].to.x - 1)
                    area->header.buffer[y][x] = C(PARTICIPANT_VERTICAL_LINE); // VERTICAL - LINE
            }
        }
        // TODO : alignment
        y = area->header.participants[m].from.y + 1 + PARTICIPANT_TOP_GAP;
        x = area->header.participants[m].from.x + 1 + PARTICIPANT_LEFT_GAP;
        char *c;
        char *tc;
        int g_c = 0;
        for (t = 0; t < participants.members[m]->name->length; t++)
        {
            if (participants.members[m]->name->string[t] == '\n')
            {
                tc = area->header.buffer[y][x + g_c + PARTICIPANT_LEFT_GAP];
                if (g_c > 0 && tc[0] != ' ')
                {
                    char *n = (char*)malloc_s(g_c * sizeof(char));
                    memset(&n[0], '\b', g_c);
                    memcpy(&n[g_c], tc, strlen(tc)+1);
                    area->header.buffer[y][x + g_c + PARTICIPANT_LEFT_GAP] = n;
                    g_c = 0;
                }
                y++;
                x = area->header.participants[m].from.x + 1 + PARTICIPANT_LEFT_GAP;
            }
            else
            {
#ifdef UTF_SUPPORT
                if (participants.members[m]->name->string[t] >> 7)
                {
                    c = (char*)malloc_s(sizeof(char)*4);
                    c[3] = '\0';
                    memcpy(&c[0], &participants.members[m]->name->string[t], 3);
                    t += 2;
                    g_c++;
                }
                else
                {
                    c = (char*)malloc_s(sizeof(char)*2);
                    c[1] = '\0';
                    c[0] = participants.members[m]->name->string[t];
                }
                area->header.buffer[y][x] = c;
#else
                area->header.buffer[y][x] = participants.members[m]->name->string[t];
#endif
                x++;
            }
        }
        tc = area->header.buffer[y][x + g_c + PARTICIPANT_LEFT_GAP];
        if (g_c > 0 && tc[0] != ' ')
        {
            char *n = (char*)malloc_s(g_c * sizeof(char));
            memset(&n[0], '\b', g_c);
            memcpy(&n[g_c], tc, strlen(tc)+1);
            area->header.buffer[y][x + g_c + PARTICIPANT_LEFT_GAP] = n;
        }
    }
}

void print_header(Area area)
{
    int x, y;
#ifdef UTF_SUPPORT
    for (y = 0; y < area.header.pos.y; y++)
    {
        if (prefix != NULL)
            printf("%s", prefix);
        for (x = 0; x < area.header.pos.x; x++)
        {
            printf("%s", area.header.buffer[y][x]);
        }
        if (suffix != NULL)
            printf("%s", suffix);
        putchar('\n');
    }
#else
    for (y = 0; y < area.header.pos.y; y++)
        if (prefix != NULL)
            printf("%s", prefix);
        printf("%s", area.header.buffer[y]);
        if (suffix != NULL)
            printf("%s", suffix);
        putchar('\n');
#endif
}

void print_body(Area area)
{
    int x, y;
#ifdef UTF_SUPPORT
    for (y = 0; y < area.body.pos.y; y++)
    {
        if (prefix != NULL)
            printf("%s", prefix);
        for (x = 0; x < area.header.pos.x; x++)
        {
            printf("%s", area.body.buffer[y][x]);
        }
        if (suffix != NULL)
            printf("%s", suffix);
        putchar('\n');
    }
#else
    for (y = 0; y < area.body.pos.y; y++)
    {
        if (prefix != NULL)
            printf("%s", prefix);
        printf("%s", area.body.buffer[y]);
        if (suffix != NULL)
            printf("%s", suffix);
        putchar('\n');
    }
#endif
}

void add_participants_line(Area *area, int size)
{
    int x, y, m;
    area->body.pos.y += size;
    area->body.pos.x = (area->body.pos.x == 0) ? area->header.pos.x : area->body.pos.x;
#ifdef UTF_SUPPORT
    area->body.buffer = (char***)realloc_s(area->body.buffer, area->body.pos.y * sizeof(char**));
#else
    area->body.buffer = (char**)realloc_s(area->body.buffer, area->body.pos.y * sizeof(char*));
#endif
    for (y = area->body.pos.y - size; y < area->body.pos.y; y++)
    {
#ifdef UTF_SUPPORT
        area->body.buffer[y] = (char**)malloc_s(area->body.pos.x * sizeof(char*));
        for (x = 0; x < area->body.pos.x; x++)
            area->body.buffer[y][x] = " ";
#else
        area->body.buffer[y] = (char*)malloc_s(area->body.pos.x * sizeof(char));
        memset(area->body.buffer[y], ' ', area->body.pos.x);
        area->body.buffer[y][area->body.pos.x] = '\0';
#endif
        for (m = 0; m < participants.members_num; m++)
        {
            area->body.buffer[y][GET_LINE_M(area->header.participants[m].from.x, area->header.participants[m].to.x)] = C(VERTICAL_LINE);
        }
    }
}

void cal_arrow_coordinates(Area *area)
{
    int a, x, y;
    for (a = 0; a < arrow_connections.cons_num; a++)
    {
        add_participants_line(area, 1);
        int from = GET_LINE_M(area->header.participants[arrow_connections.cons[a]->from->priority].from.x, area->header.participants[arrow_connections.cons[a]->from->priority].to.x);
        int to = GET_LINE_M(area->header.participants[arrow_connections.cons[a]->to->priority].from.x, area->header.participants[arrow_connections.cons[a]->to->priority].to.x);
        area->body.arrow_defs[a].from.x = from;
        area->body.arrow_defs[a].to.x = to;
        int y_size = area->body.arrow_defs[a].size.y + 2;
        area->body.l_p += y_size;
        area->body.arrow_defs[a].from.y = area->body.l_p;
        area->body.arrow_defs[a].to.y = area->body.l_p;

        // =============================================
        //  If there's nothing above current arrow, decrease y position. <- doesn't work with self assign TODO: fix here
/*         if (a != 0) */
/*         { */
/*             bool has_space = false; */
/*             int temp = 0; */
/*             for (y = 0; y < area->body.arrow_defs[a].size.y; y++) */
/*             { */
/* #ifdef UTF_SUPPORT */
/*                 if (!strcmp(area->body.buffer[area->body.l_p - y_size - y][GET_LINE_M(from, to)], " ")) */
/*                 { */
/*                     temp++; */
/*                     has_space = true; */
/*                 } */
/*                 else break; */
/* #endif */
/*             } */
/*             if (has_space) */
/*             { */
/*                 area->body.l_p -= temp - 1; */
/*                 area->body.arrow_defs[a].from.y -= temp - 1; */
/*                 area->body.arrow_defs[a].to.y -= temp - 1; */
/*                 y_size -= temp - 1; */
/*             } */
/*         } */
        // =============================================

        add_participants_line(area, y_size);
        bool left_dir = false;
        bool self_message = false;
        if (from == to)
        {
            self_message = true;
        }
        else if (from > to) // self message
        {
            left_dir = true;
            int temp = to;
            to = from;
            from = temp;
        }
        if (self_message)
        {
            add_participants_line(area, 2);
            for (y = 0; y < 3; y++) // y = 3
            {
                for (x = from; x < from + area->body.arrow_defs[a].size.x + 3; x++)
                {
                    if (x == from && y == 0)
                        area->body.buffer[area->body.l_p+y][x] = ARROW_ORIGIN_R;
                    else if (x == from + area->body.arrow_defs[a].size.x + 2 && y == 0)
                        area->body.buffer[area->body.l_p+y][x] = PARTICIPANT_TOP_RIGHT;
                    else if (x == from + area->body.arrow_defs[a].size.x + 2 && y == 2)
                        area->body.buffer[area->body.l_p+y][x] = PARTICIPANT_BOTTOM_RIGHT;
                    else if (x == from + area->body.arrow_defs[a].size.x + 2 && y == 1)
                    {
                        if (arrow_connections.cons[a]->type == (L_RET_AR_F) ||
                                arrow_connections.cons[a]->type == (R_RET_AR_F))
                            area->body.buffer[area->body.l_p+y][x] = ARROW_RET_VERTICAL_LINE;
                        else
                            area->body.buffer[area->body.l_p+y][x] = ARROW_NORMAL_VERTICAL_LINE;
                    }
                    else if (y == 2 && x == from + 1)
                    {
                        if (arrow_connections.cons[a]->type == (L_RET_AR_F) ||
                                arrow_connections.cons[a]->type == (R_RET_AR_F))
                            area->body.buffer[area->body.l_p+y][x] = ARROW_RET_L;
                        else
                            area->body.buffer[area->body.l_p+y][x] = ARROW_NORMAL_L;
                    }
                    else if (x != from && y == 0)
                    {
                        if (arrow_connections.cons[a]->type == (L_RET_AR_F) ||
                                arrow_connections.cons[a]->type == (R_RET_AR_F))
                            area->body.buffer[area->body.l_p+y][x] = ARROW_RET_LINE_R;
                        else
                            area->body.buffer[area->body.l_p+y][x] = ARROW_LINE_R;
                    }
                    else if (x != from && y == 2)
                    {
                        if (arrow_connections.cons[a]->type == (L_RET_AR_F) ||
                                arrow_connections.cons[a]->type == (R_RET_AR_F))
                            area->body.buffer[area->body.l_p+y][x] = ARROW_RET_LINE_L;
                        else
                            area->body.buffer[area->body.l_p+y][x] = ARROW_LINE_L;
                    }
                }
            }

            area->body.l_p += 2;
        }
        else
        {
            for (x = from; x < to + 1; x++)
            {
#ifdef UTF_SUPPORT
                if (left_dir)
                {
                    if (x == to)
                        area->body.buffer[area->body.l_p][x] = ARROW_ORIGIN_L;
                    else if (x == from + 1)
                    {
                        if (arrow_connections.cons[a]->type == (L_RET_AR_F)
                                || arrow_connections.cons[a]->type == (R_RET_AR_F))
                            area->body.buffer[area->body.l_p][x] = ARROW_RET_L;
                        else
                            area->body.buffer[area->body.l_p][x] = ARROW_NORMAL_L;
                    }
                    else if (x != from)
                    {
                        if (arrow_connections.cons[a]->type == (L_RET_AR_F)
                                || arrow_connections.cons[a]->type == (R_RET_AR_F))
                            area->body.buffer[area->body.l_p][x] = ARROW_RET_LINE_L;
                        else
                            area->body.buffer[area->body.l_p][x] = ARROW_LINE_L;
                    }
                }
                else
                {
                    if (x == from)
                        area->body.buffer[area->body.l_p][x] = ARROW_ORIGIN_R;
                    else if (x == to)
                    {
                        if (arrow_connections.cons[a]->type == (L_RET_AR_F)
                                || arrow_connections.cons[a]->type == (R_RET_AR_F))
                            area->body.buffer[area->body.l_p][x-1] = ARROW_RET_R;
                        else
                            area->body.buffer[area->body.l_p][x-1] = ARROW_NORMAL_R;
                    }
                    else
                    {
                        if (arrow_connections.cons[a]->type == (L_RET_AR_F)
                                || arrow_connections.cons[a]->type == (R_RET_AR_F))
                            area->body.buffer[area->body.l_p][x] = ARROW_RET_LINE_R;
                        else
                            area->body.buffer[area->body.l_p][x] = ARROW_LINE_R;
                    }
                }
#else
                area->body.buffer[area->body.l_p][x] = '-';
#endif
            }

        }
    }
}

void cal_arrow_message_size(Area *area)
{
    int a;

    area->body.arrow_defs = (struct arrow_def_coor*)malloc_s(sizeof(struct arrow_def_coor) * arrow_connections.cons_num);
#ifdef DEBUG
    printf("arrow num: %d\n", arrow_connections.cons_num);
#endif
    for (a = 0; a < arrow_connections.cons_num; a++)
    {
        area->body.arrow_defs[a].size = get_word_size(arrow_connections.cons[a]->content, (int)strlen(arrow_connections.cons[a]->content));

        area->body.arrow_defs[a].top_padding = area->body.arrow_defs[a].size.y + 1;

        int width = area->header.participants[arrow_connections.cons[a]->to->priority].middle - area->header.participants[arrow_connections.cons[a]->from->priority].middle;
        int *from;
        if (width < 0)
            from = &area->header.participants[arrow_connections.cons[a]->to->priority].right_padding;
        else
            from = &area->header.participants[arrow_connections.cons[a]->from->priority].right_padding;
        width = abs(width);
        bool self_arrow = false;
        if (width == 0)
        {
            self_arrow = true;
            width = area->body.arrow_defs[a].size.x;
        }

        if (width < area->body.arrow_defs[a].size.x + 3)
        {
            //TODO: working on here
            int temp = 0;
#ifdef DEBUG
            printf("cc:%d\n", area->body.arrow_defs[a].size.x);
#endif
            if (self_arrow)
            {
                int klp = (area->header.participants[arrow_connections.cons[a]->to->priority].middle - area->header.participants[arrow_connections.cons[a]->to->priority].from.x);
                if (arrow_connections.cons[a]->from->priority == participants.members_num - 1) // last arrow of self
                    temp = width - klp + 2;
                else
                {
                    int kj = 0;
                    if (arrow_connections.cons[a]->to->priority != arrow_connections.cons_num - 1)
                        kj = (area->header.participants[arrow_connections.cons[a]->to->priority+1].middle - area->header.participants[arrow_connections.cons[a]->to->priority+1].from.x);
                    temp = width+2 - klp;
                    temp -= kj - 2;
                }
            }
            else
                temp = area->body.arrow_defs[a].size.x - width + 3;
            if (temp > *from)
            {
                *from = temp;
            }
#ifdef DEBUG
            printf("ss:%d\n", area->body.arrow_defs[a].size.x);
#endif
        }
#ifdef DEBUG
        printf("arrow message size : %d, %d, %d\n", area->body.arrow_defs[a].size.x, area->body.arrow_defs[a].size.y, width);
#endif
    }
}

void add_arrow_messages(Area *area)
{
    int a, x = 0, y;
    bool print_at_middle = false;
    char *c;
    int dir;
    for (a = 0; a < arrow_connections.cons_num; a++)
    {
        int g_c = 0;
        print_at_middle = false;
        if (abs(area->body.arrow_defs[a].from.x - area->body.arrow_defs[a].to.x) - 4 > area->body.arrow_defs[a].size.x)
        {
            print_at_middle = true;
            x = (abs(area->body.arrow_defs[a].from.x - area->body.arrow_defs[a].to.x) / 2) - (area->body.arrow_defs[a].size.x / 2) - 2;
        }
        y = area->body.arrow_defs[a].size.y;
        for (int w = 0; w < strlen(arrow_connections.cons[a]->content); w++)
        {
            if (arrow_connections.cons[a]->content[w] == '\n')
            {
                c = area->body.buffer[area->body.arrow_defs[a].from.y - y][dir + 1 + x + g_c];
                if (g_c > 0 && c[0] == ' ')
                {
                    char *n = (char*)malloc_s(g_c * sizeof(char));
                    memset(&n[0], '\b', g_c);
                    memcpy(&n[g_c], c, strlen(c)+1);
                    area->body.buffer[area->body.arrow_defs[a].from.y - y][dir + 1 + x + g_c + 1] = n;
                    g_c = 0;
                }
                y--;
                if (print_at_middle)
                    x = (abs(area->body.arrow_defs[a].from.x - area->body.arrow_defs[a].to.x) / 2) - (area->body.arrow_defs[a].size.x / 2) - 2;
                else
                    x = 0;
            }
            else
            {
                dir = area->body.arrow_defs[a].from.x;
                if (dir > area->body.arrow_defs[a].to.x)
                    dir = area->body.arrow_defs[a].to.x;
                x++;
#ifdef UTF_SUPPORT
                if (arrow_connections.cons[a]->content[w] >> 7)
                {
                    c = (char*)malloc_s(sizeof(char)*4);
                    c[3] = '\0';
                    memcpy(&c[0], &arrow_connections.cons[a]->content[w], 3);
                    w += 2;
                    g_c++;
                }
                else
                {
                    c = (char*)malloc_s(sizeof(char)*2);
                    c[1] = '\0';
                    c[0] = arrow_connections.cons[a]->content[w];
                }
                area->body.buffer[area->body.arrow_defs[a].from.y - y][dir + 1 + x] = c;
#endif
            }
        }
        c = area->body.buffer[area->body.arrow_defs[a].from.y - y][dir + 1 + x + g_c];
        if (g_c > 0 && c[0] == ' ')
        {
            char *n = (char*)malloc_s(g_c * sizeof(char));
            memset(&n[0], '\b', g_c);
            memcpy(&n[g_c], c, strlen(c)+1);
            area->body.buffer[area->body.arrow_defs[a].from.y - y][dir + 1 + x + g_c + 1] = n;
        }
        y = 0;
        x = 0;
    }
}

void render()
{

    // initialize
    Area new_area = init();

    cal_coordinates_of_participants(&new_area);

    cal_arrow_message_size(&new_area);

    recal_coordinates_of_participants(&new_area);
    add_participants_to_buffer(&new_area);

    cal_arrow_coordinates(&new_area);
    add_arrow_messages(&new_area);

    int l = new_area.header.pos.x;
    if (prefix != NULL)
        l += strlen(prefix);
    if (suffix != NULL)
        l += strlen(suffix);

    char a[l];
    if (!printRaw) {
        memset(a, '=', l);
        a[l] = '\0';
        printf(KGRN"%s\n"RESET, a);
    }

    print_header(new_area);
    print_body(new_area);

    if (!printRaw) {
        printf(KGRN"%s\n"RESET, a);
    }
}
