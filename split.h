#ifndef SPLIT_H
#define SPLIT_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mem.h"
#include "terminal.h"

#define SET_WORD_TYPE_AND_RET(word, thistype) \
    int string_size = strlen(word->string); \
    if (!word->init && string_size) \
    { \
        word->length = string_size; \
        word->init = true; \
    } \
    if (string_size) \
        word->type = thistype; \
    return string_size

#define TEXT_WRAP_SYM '\"'
// #define TEXT_WRAP_SYM '\''
#define EOA '\0'
#define UNINITIALIZED -1

#define TEXT_STRING 0
//TODO: color syntax

typedef struct
{
    char rgb[3][2];
} Color;

typedef enum {
    Uninitialized = -1,
    Unknown       = 0b00,
    Plain         = 0b01,
    Statement     = 0b10,
    Symbol        = 0b11,
} Type;

typedef struct
{
    bool init;
    int length;
    char *string;
    bool has_alias;
    char *alias;
    int l_spc;
    int f_spc;
    Type type;
    uint32_t d_type;
} Word;

typedef union
{
    char *c_ptr;
    bool init;
} c_i;

static inline Word* new_word_p(char *str, int size)
{
    bool init = true;
    Type type = Uninitialized;
    if (!str[0] && !size) init = false;
    if (!str[0] && size)
    {
        init = false;
        str = (char *)calloc_s(size, sizeof(char));
    }
    if (str[0] && !size) size = strlen(str);
    Word *n_word = (Word *)calloc_s(1, sizeof(Word));
    n_word->init = init;
    n_word->type = type;
    n_word->length = size;
    n_word->l_spc = 0;
    n_word->has_alias = false;
    n_word->string = strdup(str);
    n_word->d_type = 0;
    return n_word;
}

static inline void inc_word(Word *target, int len)
{
    int t = target->length;
    target->length += len;
    target->string = (char*)realloc_s(target->string, sizeof(char) * target->length);
    memset(target->string+t, 0, len);
}

static inline Word* merge_words(int count, Word *words[], bool withspc)
{
    Word *merged_words = new_word_p((char*)"", 0);
    char *bufw;
    for (int i = 0; i < count; i++)
    {
        bufw = (char*)calloc_s(words[i]->length, sizeof(char));
        strcpy(bufw, words[i]->string);
        bufw[words[i]->length] = '\0';
#ifdef _DEBUG
        printf("w:%s, s:%d, r:%lu, lp:%d\n", bufw, words[i]->length, strlen(bufw), words[i]->l_spc);
#endif
        inc_word(merged_words, words[i]->length + words[i]->l_spc);
        strncat(merged_words->string, words[i]->string, words[i]->length);
        free(bufw);
        if (withspc && words[i]->l_spc)
        {
            bufw = (char*)malloc_s(words[i]->l_spc);
            memset(bufw, ' ', words[i]->l_spc);
            bufw[words[i]->l_spc] = '\0';
            strcat(merged_words->string, bufw);
            free(bufw);
        }
    }
    if (strlen(merged_words->string) != merged_words->length)
        merged_words->string[merged_words->length] = '\0';
    return merged_words;
}

typedef struct
{
    int lp;
    Word **c;
    uint32_t ewp; // each words position
    bool is_eol;
    // 0x '00' '00' '00' '00', each '00' means a word,
    // 01: plain, 10: statement, 11: symbol, 00: something else(this could be an empty)
    // When it's something else, check inside of the word (d_type).
} Words;

#include "grammars/statement.h"

static inline Words* connect_words(Words* prev, Words* cur)
{
    inc_word(prev->c[prev->lp - 1], 1);
    prev->c[prev->lp - 1]->string[prev->c[prev->lp - 1]->length - 1] = '\n';
    Word* words_to_merge[] = { prev->c[prev->lp - 1], cur->c[0] };
    prev->c[prev->lp - 1] = merge_words(2, words_to_merge, false);
    prev->c[prev->lp - 1]->type = Plain;
    prev->c[prev->lp - 1]->d_type = WRAPPED_WORD;
    prev->is_eol = cur->is_eol;
    int lp_b = prev->lp;
#ifdef DEBUG
    printf("prev : %d, cur : %d, lp_b : %d, merged word: %s\n", prev->lp, cur->lp, lp_b, prev->c[prev->lp-1]->string);
#endif
    prev->lp += cur->lp - 1;
    if (prev->lp != lp_b)
        prev->c = (Word **)realloc_s(prev->c, sizeof(Word) * (prev->lp + 1));
    for (int _i = 1; _i < cur->lp; _i++)
    {
#ifdef DEBUG
        printf("_i : %d, new_cur : %d\n", _i, lp_b + _i - 1);
#endif
        prev->c[lp_b + _i - 1] = cur->c[_i];
    }
    free(cur->c[0]->string);
    free(cur->c[0]);
    free(cur);
    return prev;
}

static inline Words* new_words()
{
    Words* n_words = (Words*)calloc_s(1, sizeof(Words));
    n_words->lp = 0;
    n_words->ewp = 0;
    n_words->is_eol = 0;
    return n_words;
}

#define COPY_TO_WORDS(words, word) \
    words->lp++; \
    words->c = (Word **)realloc_s(words->c, sizeof(Word) * (words->lp)); \
    words->c[words->lp - 1] = word; \

static inline void drop_word_from_words(Words *words, int from, int to)
{
    for (int _i = from; _i < words->lp && _i < to; _i++)
    {
        free(words->c[_i]->string);
        free(&words->c[_i]);
    }
    words->lp -= to - from;
}

static inline void dump_words(Words* words, int i)
{
    int _i, _j;
    for (_i = 0; _i < words->lp; _i++)
    {
        for (_j = 0; _j < words->c[_i]->f_spc; _j++)
            putchar(' ');
        if (i == _i)
            printf(KYEL "%s" RESET, words->c[_i]->string);
        else
            printf("%s", words->c[_i]->string);
        for (_j = 0; _j < words->c[_i]->l_spc; _j++)
            putchar(' ');
    }
    putchar('\n');
    printf(KRED);
    for (_i = 0; _i < words->lp; _i++)
    {
        for (_j = 0; _j < words->c[_i]->f_spc; _j++)
            putchar(' ');
        for (_j = 0; _j < words->c[_i]->length; _j++)
        {
            if (i == _i && !_j)
                putchar('^');
            else if (i == _i)
                putchar('~');
            else
                putchar(' ');
        }
        for (_j = 0; _j < words->c[_i]->l_spc; _j++)
            putchar(' ');
    }
    printf(RESET);
    putchar('\n');
}

#endif
