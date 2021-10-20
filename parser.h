#include "scanner.h"
#include "mem.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "terminal.h"

#ifdef CLI
extern int SHOW_LOG;
#endif

#ifdef PYTHON_BINDING
extern char** PY_list_result;
extern int result_list_size;
extern int failed_at;
extern int chunk_size;
extern int buffer_size;
#endif

Words* split(char *line);
bool parse(Words *words, int line_num);

inline static void parse_line(char *line, int line_num)
{
    // printf("==========================\n - parsing \"\"\" %s \"\"\"\n", line);
    Words* newline = split(line);
    if (newline != NULL)
    {
        bool result = parse(newline, line_num);

#ifdef CLI
        if (SHOW_LOG >= 1 && result)
        {
            printf_c(KBLU " - line parsed.\n");
        }
#endif
    }
    else
    {
#ifdef PYTHON_BINDING
        failed_at = line_num;
#else
        printf(KRED"**** PARSE FAILED at line %d ****\n"RESET, line_num);
#endif
    }
}

#ifdef PYTHON_BINDING
inline static void clear_temps()
{
    // ----------------------------
    // TODO: clear dangling pointer
    // ----------------------------
    arrow_connections.cons_num = 0;
    participants.members_num = 0;
    PY_list_result = NULL;
    // for (int i = 0; i < result_list_size; i++)
    //     free(PY_list_result[i]);
    // free(PY_list_result);
    result_list_size = 0;
    failed_at = -1;
    chunk_size = 0;
    buffer_size = 0;
}
#endif
