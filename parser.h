#include "scanner.h"
#include "mem.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "terminal.h"

extern int SHOW_LOG;

Words* split(char *line);
bool parse(Words *words, int line_num);

inline static void parse_line(char *line, int line_num)
{
    // printf("==========================\n - parsing \"\"\" %s \"\"\"\n", line);
    Words* newline = split(line);
    bool result = parse(newline, line_num);

    if (SHOW_LOG >= 1 && result)
    {
        printf_c(KBLU " - line parsed.\n");
    }
}
