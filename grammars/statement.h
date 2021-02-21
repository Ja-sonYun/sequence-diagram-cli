#ifndef STATEMENT_H
#define STATEMENT_H

#include "../split.h"
#include "../mem.h"
//  'participant' | string | 'as' | word
#define PARTICIPANT_AS_BIN             0b01100110
//  'participant' | word
#define PARTICIPANT_BIN                0b0110

//  word | symbol | word | symbol | string
#define ARROW_CONNECTION_BIN           0b0111011101


#define ST(BIN)         (BIN ^ (0b01 << 28))
#define ST_MASK         0b00100000000000000000000000000000
//                               ^       ^       ^       ^
#define IS_ST(BIN)      (BIN | ST_MASK)
static const int _statements_s = 3;
static const struct _Statements {
    char name[20];
    uint32_t o;
} _statements[] = {
#define WRAPPED_WORD     ST(0x00000001)
// *** participant ***
#define PARTICIPANT_F    ST(0x00000003)
#define PARTICIPANT      "participant"
    { PARTICIPANT, PARTICIPANT_F },
// *** as ***
#define AS_F             ST(0x00000004)
#define AS               "as"
    { AS, AS_F },
// *** note ***
#define NOTE_F           ST(0x00000005)
#define NOTE             "note"
    { NOTE, NOTE_F },
#define OVER_F           ST(0x00000006)
#define OVER             "over"
    { OVER, OVER_F },
#define LEFT_F           ST(0x00000007)
#define LEFT             "left"
    { LEFT, LEFT_F },
#define RIGHT_F          ST(0x00000008)
#define RIGHT            "right"
    { RIGHT, RIGHT_F },

// *** arrow_connection ***
#define ARROW_CONNCETION_F ST(0x00000006)
// #define ARROW_CONNCETION   "sd"
//     { ARROW_CONNCETION, ARROW_CONNCETION_F }

};

static inline void check_statement(Word *word)
{
    for (int _i = 0; _i < _statements_s; _i++)
    {
        if (!strcmp(word->string, _statements[_i].name))
        {
            word->type = Statement;
            word->d_type = _statements[_i].o;
        }
    }
}

#define AS_BIN           0b011001

static inline void preprocessing(Words *words)
{
    return ;
    uint32_t as_b = AS_BIN;
// #define DEBUG_
#ifdef DEBUG_
    print_uint32_t("words->ewp:", words->ewp);
#endif
    for (int i = 1; i < words->lp-1; i++)
    {
#ifdef DEBUG_
        print_uint32_t("as_b      :", as_b);
        print_uint32_t("fods      :", words->ewp & as_b);
#endif
        if (as_b == (words->ewp & as_b) && words->c[i]->d_type == AS_F && words->c[i-1]->d_type == WRAPPED_WORD && words->c[i+1]->d_type != WRAPPED_WORD)
        {
            words->c[i+1]->has_alias = true;
            words->c[i+1]->alias = words->c[i-1]->string;
            uint32_t mask = 0b001111 << i*2;
            words->ewp &= ~mask;
            // clean up
            printf(" - %s\n", words->c[i]->string);
            // free((char*)words->c[i]->string); // free 'as'
            // free(&words->c[i]);       // free word:as
            // printf(" ere\n");
            // free(&words->c[i-1]);     // free word:' '
            // realign
            // for (int j = i+1; j < words->lp; j++)
            // {
            //     printf(" - move %s to %s\n", words->c[j].string, words->c[j-2].string);
            //     words->c[j] = words->c[j-2];
            // }
            // words->lp -= 2; i -= 2;
            // words->c = (Word *)realloc_s(words->c, sizeof(Words) * words->lp);
#ifdef DEBUG_
            print_uint32_t("masked    :", words->ewp);
#endif
        }
        as_b <<= 2;
    }
}
#endif
