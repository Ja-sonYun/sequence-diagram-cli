#include "parser.h"

static int max_call_lim = 0;
static inline void watch_compile_error(int size)
{
    if (max_call_lim > size)
    {
        printf("\ncompile error!\n");
        exit(0);
    };
    max_call_lim++;
}

// EP
Words* split(char *line)
{
    max_call_lim = 0;
    static bool nl = false;
    static bool wrapped = false;
    int w_size;
    /* bool may_empty = false; */
    if (nl) nl = false;

    Words* line_temp = new_words();

    Word* word = new_word_p("", 0);

    for (int len = 0; len < strlen(line); len++)
    {
        watch_compile_error(strlen(line));
        if (wrapped) goto STILL_WRAPPED;
        switch (line[len])
        {
            case '\\':
                nl = true;
                /* return; */

            case ' ':
            case '\t':
                break;

            case TEXT_WRAP_SYM:
                wrapped = true;
                len++; // go inside of symbols
STILL_WRAPPED:
                w_size = rdw_ignspc(word, &line[len], &wrapped);
                if (w_size)
                {
                    COPY_TO_WORDS(line_temp, word);
                    word = new_word_p("", 0);
                }
                len += w_size - 1;
                if (!wrapped)
                    len++; // escape wrapped string
                break;

            default:
                if (is_sym(line[len]))
                {
                    w_size = rd_sym(word, &line[len]);
                    if (w_size)
                    {
                        COPY_TO_WORDS(line_temp, word);
                        word = new_word_p("", 0);
                    }
                    len += w_size - 1;
                }
                else
                {
                    w_size = rdw(word, &line[len]);
                    int temp_p = w_size + len;
                    word->l_spc = 0;
                    while (line[temp_p] == ' ')
                    {
                        temp_p++;
                        word->l_spc++;
                    }
                    if (w_size)
                    {
                        if (strcmp(word->string, " "))
                        {
                            COPY_TO_WORDS(line_temp, word);
                        }
                        word = new_word_p("", 0);
                    }
                    len += w_size - 1;
                }
        }
    }
    if (wrapped)
        line_temp->is_eol = true;
    return line_temp;
}

bool parse(Words *words, int line_num)
{
    static Words* prev_words;
    static bool still_wrapped = false;
#ifdef DEBUG
    printf("is wrapped : %d\n", words->is_eol);
#endif
    if (still_wrapped)
    {
        // merge prev_words[-1] cur_words[0]
#ifdef DEBUG
        printf("merge -> '%s' : '%s' \n", prev_words->c[prev_words->lp-1]->string, words->c[words->lp-1]->string);
#endif
        words = connect_words(prev_words, words);
#ifdef DEBUG
        printf("merged. '%s'\n", words->c[words->lp - 1]->string);
#endif
        still_wrapped = false;
        prev_words = NULL;
    }
    if (words->is_eol)
    {
        prev_words = words;
        still_wrapped = true;
        return false;
    }
    for (int i = 0; i < words->lp; i++)
    {
        // search and set each words roll
        Type temp_T = words->c[i]->type;
        if (temp_T == Uninitialized)
            temp_T = Unknown;
        words->ewp ^= temp_T << (i * 2);
#ifdef DEBUG
        printf(" - word : %s, type : %d, d_type : %d, size : %d, r_size : %lu, spc : %d\n", words->c[i]->string, words->c[i]->type, words->c[i]->d_type, words->c[i]->length, strlen(words->c[i]->string), words->c[i]->l_spc);
        print_uint32_t("ewp : ", words->ewp);
#endif
    }
#ifdef DEBUG
    print_uint32_t("ewp : ", words->ewp);
#endif
    preprocessing(words); // '"A" as a' to single word

    if (check_is_participant(words) ||
        check_is_arrow(words))
        return true;

    printf(KRED"**** PARSE FAILED at line %d ****\n"RESET, line_num);
    return false;
}
