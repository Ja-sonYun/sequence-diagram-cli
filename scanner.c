#include "scanner.h"

/* #define  DEBUG */
static inline void debug(char *str)
{
#ifdef DEBUG
        printf(" - %s\n", str);
#endif
}

static char _SYMS[] = { '|', '\\', '-', '>', '<', 'x', ':', '.' };
static int _SYMS_S = sizeof(_SYMS) / sizeof(char);
static char _NOT_ALLOWED[] = { ' ', '\0', TEXT_WRAP_SYM };
static int  _NOT_ALLOWED_S = sizeof(_NOT_ALLOWED) / sizeof(char);

// is symbol
bool is_sym(char ch)
{
    int i = 0;
    while (i < _SYMS_S)
    {
        if (ch == _SYMS[i])
            return true;
        i++;
    }
    return false;
}

bool _is_word(char ch)
{
    bool is_ALLOWED = true;
    bool is_SYM = true;
    for (int i = 0; i < _NOT_ALLOWED_S; i++)
    {
        if (ch == _NOT_ALLOWED[i])
            is_ALLOWED = false;
    }
    for (int i = 0; i < _SYMS_S; i++)
    {
        if (ch == _SYMS[i])
            is_SYM = false;
    }
    return is_ALLOWED && is_SYM;
}

bool _is_closed(char ch)
{
    return ch != TEXT_WRAP_SYM;
}

void _read_w(Word *word, char *ch, bool (*stopper)(char), Type type)
{
    debug("_read_w entered");
    bool is_wrapped = false;
    int i = 0;
    while (stopper(ch[i]))
    {
        if (ch[i] == EOA)
        {
            debug("return empty");
            is_wrapped = true;
            break;
        }
        i++;
    }

    if (!word->init)
    {
        char temp[i];
        strncpy(temp, ch, i);
        if (!word->length)
            word->length = i - word->l_spc;
        word->string = (char*)malloc_s(sizeof(char) * word->length);
        strncpy(word->string, temp, word->length);
        word->string[word->length] = '\0';
        word->type = is_wrapped ? Uninitialized : type;
        word->init = !is_wrapped;
    }
}


// read word ignore space
int rdw_ignspc(Word *word, char *ch, bool *wrapped)
{
    debug("called rdw_ignspc()");
    _read_w(word, ch, _is_closed, Plain);
    debug(word->string);
    if (word->type != Uninitialized)
    {
        if (word->length != strlen(word->string))
            word->string[word->length] = '\0';
        *wrapped = false;
    }
    word->d_type = WRAPPED_WORD;
    debug("return rdw_ignspc()");
    return word->length;
}

int rd_sym(Word *l_sym, char *ch)
{
    debug("called rd_sym()");
    _read_w(l_sym, ch, is_sym, Symbol);
    check_symbol(l_sym);
    debug(l_sym->string);
    debug("return rd_sym()");
    return l_sym->length;
}

// read word
// - return last ch pointer as int
int rdw(Word *curw, char *ch)
{
    debug("called rdw()");
    _read_w(curw, ch, _is_word, Plain);
    check_statement(curw);
    debug(curw->string);
    debug("return rdw()");
    return curw->length;
}
