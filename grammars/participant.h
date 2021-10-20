#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <stdbool.h>
#include "../mem.h"
#include "../split.h"
#include "statement.h"

#include "../terminal.h"

#define IS_PARTICIPANT_LINE(words) \
    (words->ewp == (UNFLAG_R(words->ewp, 4) | PARTICIPANT_BIN) \
                && words->c[0]->d_type == PARTICIPANT_F)

// same participants, with header icon
#define PARTICIPANT_TYPE_ACTOR 1
#define PARTICIPANT_TYPE_BOUNDARY 2
#define PARTICIPANT_TYPE_CONTROL 3
#define PARTICIPANT_TYPE_ENTIRY 4
#define PARTICIPANT_TYPE_DATABASE 5
#define PARTICIPANT_TYPE_COLLECTIONS 6
#define PARTICIPANT_START_SYM 'participant'

#ifdef CLI
extern int SHOW_LOG;
#endif

typedef struct
{
    unsigned int icon : 1;
    bool has_alias;
    Word *name;
    Word *alias_name;
    int priority;
    uint8_t alignment;
    Color color;
} Participant;

typedef struct
{
    int members_num;
    Participant **members;
} Participants;

extern Participants participants;

static inline void join_participants(Participant* new_member)
{
    participants.members_num++;
    participants.members = (Participant**)realloc_s(participants.members, sizeof(Participant) * participants.members_num);
    participants.members[participants.members_num - 1] = new_member;
#ifdef CLI
    if (SHOW_LOG >= 1)
    {
        if (new_member->has_alias)
        {
            printf_c(KCYN " * ");
            printf("register new participant with alias: '%s' as %s , priority: '%d'\n", new_member->name->string, new_member->alias_name->string, participants.members_num);
        }
        else
        {
            printf_c(KCYN " * ");
            printf("register new participant: '%s' , priority: '%d'\n", new_member->name->string, participants.members_num);
        }
    }
#endif
}

static inline Participant *find_participant(char *str)
{
    for (int _i = 0; _i < participants.members_num; _i++)
    {
        if ((participants.members[_i]->has_alias && !strcmp(participants.members[_i]->alias_name->string, str))
                || !strcmp(participants.members[_i]->name->string, str))
        {
            return participants.members[_i];
        }
    }
    return NULL;
}

                //( words->lp == 2 || ( words->lp == 4 && words->c[1].d_type & WRAPPED_WORD
                  //                    && words->c[2].d_type & AS_F ) ))
static inline bool check_is_participant(Words *words)
{
    if (IS_PARTICIPANT_LINE(words)) // checking participant exist
    {
        bool has_alias;

        if (words->lp == 2 && !(words->c[1]->d_type == WRAPPED_WORD))
            has_alias = false;
        else if (words->lp == 4 && words->c[1]->d_type == WRAPPED_WORD && words->c[2]->d_type == AS_F) // checking "A" as a
            has_alias = true;
        else
        {
            // error message
            if (words->lp > 3 && !(words->c[1]->d_type == WRAPPED_WORD) )
            {
                printf_c(KRED "ERROR");
                printf(": replace '%s' to ' \"%s\" '. \n", words->c[1]->string, words->c[1]->string);
                dump_words(words, 1);
            }
            else if (words->lp == 2 && (words->c[1]->d_type == WRAPPED_WORD))
            {
                printf_c(KRED "ERROR");
                printf(": Replace to 'participant '%s' as $(ALIAS)'!\n", words->c[1]->string);
                dump_words(words, 1);
            }
            else
            {
                printf_c(KRED "ERROR");
                printf(": Something wrong!\n");
            }

            return false;
        }

        if ((!has_alias && find_participant(words->c[1]->string))
                || (has_alias && find_participant(words->c[3]->string)))
        {
            printf_c(KRED "ERROR");
            printf(": '%s' already registered!\n", has_alias ? words->c[3]->string : words->c[1]->string);
            return false;
        }

        // when executed without any error
        Participant *new_participant = (Participant *)calloc_s(1, sizeof(Participant));
        new_participant->has_alias = has_alias;
        new_participant->alias_name = has_alias ? words->c[3] : NULL;
        new_participant->priority = participants.members_num;
        new_participant->name = words->c[1];
        join_participants(new_participant);
        return true;
    }
    return false;
}

#endif
