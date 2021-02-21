#ifndef SYMBOL_H
#define SYMBOL_H
#include <stdlib.h>
#include "../split.h"

// Symbols
#define SYM(BIN)              BIN ^ (0b01 << 30)
#define SYMBOL_MASK           0b10000000000000000000000000000000
#define IS_SYM(BIN)           (BIN | SYMBOL_MASK)
static const struct _Symbols {
    char *name;
    uint32_t flag; // use last bit for direction.
} _symbols[] = {
// Arrow styles
#define ARROW_MASK            0b01000000000000000000000000000000
#define IS_ARROW(BIN)         (BIN | ARROW_MASK)
#define ARROW(BIN)            BIN ^ ARROW_MASK
#define LEFT_DIR_MASK         0b00100000000000000000000000000000
#define LEFT_DIR(ARROW_F)     ARROW_F ^ LEFT_DIR_MASK
#define IS_LEFT_DIR(ARROW_F)  ARROW_F & LEFT_DIR_MASK
#define UNSET_DIR(ARROW_F)    ARROW_F & 0b11011111111111111111111111111111
#define R_ARROW_F             SYM(ARROW(0x00000001))
#define R_ARROW               "->"
    { R_ARROW, R_ARROW_F },
#define L_ARROW_F             LEFT_DIR(R_ARROW_F)
#define L_ARROW               "<-"
    { L_ARROW, L_ARROW_F },
#define R_EMP_AR_F            SYM(ARROW(0x00000002))
#define R_EMP_AR              "->>"
    { R_EMP_AR, R_EMP_AR_F },
#define L_EMP_AR_F            LEFT_DIR(R_EMP_AR_F)
#define L_EMP_AR              "<<-"
    { L_EMP_AR, L_EMP_AR_F },
#define R_RET_AR_F            SYM(ARROW(0x00000003))
#define R_RET_AR              "-->"
    { R_RET_AR, R_RET_AR_F },
#define L_RET_AR_F            LEFT_DIR(R_RET_AR_F)
#define L_RET_AR              "<--"
    { L_RET_AR, L_RET_AR_F },

    //===========================================
#define STYLE_MASK            0b00010000000000000000000000000000
#define IS_STYLE(BIN)         (BIN | STYLE_MASK)
#define STYLE(BIN)            BIN ^ STYLE_MASK

#define DELAY_F               SYM(STYLE(0x00000001))
#define DELAY                 "..."
    { DELAY, DELAY_F },
#define SPACE_F               SYM(STYLE(0x00000002))
#define SPACE                 "|||"
    { SPACE, SPACE_F },
#define SEPERATER_F           SYM(STYLE(0x00000003))
#define SEPERATER             "=="
    { SEPERATER, SEPERATER_F },
};
static const int _symbols_s = 6;

static inline char* find_symbol(uint32_t F)
{
    for (int _i = 0; _i < _symbols_s; _i++)
    {
        if (_symbols[_i].flag == F)
        {
            return _symbols[_i].name;
        }
    }
    return NULL;
}

static inline void check_symbol(Word *word)
{
    for (int _i = 0; _i < _symbols_s; _i++)
    {
        if (!strcmp(_symbols[_i].name, word->string))
        {
            word->d_type = _symbols[_i].flag;
        }
    }
}
#endif
