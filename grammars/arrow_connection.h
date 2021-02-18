#ifndef AROROWCONNECTION_H
#define AROROWCONNECTION_H

#include <stdbool.h>
#include "../mem.h"
#include "../split.h"
#include "participant.h"
#include "statement.h"
#include "symbol.h"

#include "../terminal.h"

extern int SHOW_LOG;

// word -> word : string
#define IS_ARROW_LINE(words) \
    (words->ewp == (UNFLAG_R(words->ewp, 10) | ARROW_CONNECTION_BIN) \
        && IS_ARROW(words->c[1]->d_type) \
        && *words->c[3]->string == ':') \

typedef struct {
    Participant *from;
    Participant *to;
    char *content;
    int number;
    Color color;
    uint32_t type;
} ArrowConnection;

typedef struct {
    ArrowConnection **cons;
    int cons_num;
} ArrowConnections;

extern ArrowConnections arrow_connections;

static inline void define_new_arrow(Words *words)
{
    ArrowConnection *new_arrow = (ArrowConnection *)calloc_s(1, sizeof(ArrowConnection));
    // strcpy(new_arrow->type, words->c[1]->string);
    new_arrow->type = words->c[1]->d_type;

    Participant *exist_first = find_participant(words->c[0]->string);
    if (!exist_first)
    {
        exist_first = (Participant *)calloc_s(1, sizeof(Participant));
        exist_first->has_alias = false;
        exist_first->priority = participants.members_num;
        exist_first->name = words->c[0];
        join_participants(exist_first);
    }

    Participant *exist_second = find_participant(words->c[2]->string);
    if (!exist_second)
    {
        exist_second = (Participant *)calloc_s(1, sizeof(Participant));
        exist_first->has_alias = false;
        exist_second->priority = participants.members_num;
        exist_second->name = words->c[2];
        join_participants(exist_second);
    }
    if (IS_LEFT_DIR(words->c[1]->d_type))
    {
        new_arrow->to = exist_first;
        new_arrow->from = exist_second;
    }
    else
    {
        new_arrow->from = exist_first;
        new_arrow->to = exist_second;
    }
    new_arrow->content = words->c[4]->string;

    arrow_connections.cons_num++;
    arrow_connections.cons = (ArrowConnection**)realloc_s(arrow_connections.cons, sizeof(ArrowConnection) * arrow_connections.cons_num);
    arrow_connections.cons[arrow_connections.cons_num - 1] = new_arrow;

    if(SHOW_LOG >= 1)
    {
        printf_c(KCYN " * ");
        printf("define new arrow: '%s' %d '%s' : %s\n", new_arrow->from->name->string, new_arrow->type, new_arrow->to->name->string, new_arrow->content);

    }
}

static inline bool check_is_arrow(Words *words)
{
    if (IS_ARROW_LINE(words))
    {
        static const int REAL_WORD_OFFSET = 4; // A(0) ->(1) B(2) :(3) something(4)

        if (!(words->c[REAL_WORD_OFFSET]->d_type == WRAPPED_WORD)) // check ^something is wrapped
        {                                                        // if wrapped, merge all words after ':'
            int wc_to_merge = words->lp - REAL_WORD_OFFSET;
            int i;
            Word *mergew[wc_to_merge];
            for (i = REAL_WORD_OFFSET; i < words->lp; i++)
            {
                mergew[i-REAL_WORD_OFFSET] = words->c[i];
            }
            Word *re = merge_words(wc_to_merge, mergew, true);
            words->c[REAL_WORD_OFFSET] = re;
            // resize words
            for (i = 0; i < words->lp - REAL_WORD_OFFSET; i++)
            {
                if (mergew[i]->string != NULL)
                    free(mergew[i]->string);
                if (mergew[i] != NULL)
                    free(mergew[i]);
            }
            words->c = (Word **)realloc_s(words->c, sizeof(Word) * REAL_WORD_OFFSET + 1);
            words->lp = REAL_WORD_OFFSET;
        }
        else if (words->lp > REAL_WORD_OFFSET+1 && words->c[REAL_WORD_OFFSET]->d_type == WRAPPED_WORD)
        {
            printf_c(KYEL "WARNING");
            printf(": words after : '%s' will be ignored.\n", words->c[4]->string);
        }
        define_new_arrow(words);
        return true;
    }
    return false;
}

#endif
