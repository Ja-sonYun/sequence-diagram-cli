#include "main.h"
#define STDIN_OPTION_POSITION 1
#define STANDARD_OPTION_POSITION 2
void read_lines_and_render(FILE *fp)
{
    int CUR_LINE_NUM = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        CUR_LINE_NUM++;
        if (strcmp(line, "\n") && line[0] != ';')
        {
#ifdef CLI
            if (SHOW_LOG > 1)
            {
                int line_s = strlen(line) + 3;
                char sep[line_s];
                memset(sep, '-', line_s);
                sep[line_s] = '\0';
                printf(KGRN"%s\n", sep);
                printf(KBLU" +"KCYN" %s\n"RESET, line);
            }
#endif
            line[strlen(line)-1] = '\0';
            parse_line(line, CUR_LINE_NUM);
        }
    }

    render();
    fclose(fp);
    if (line)
        free(line);
}


void readfile(char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
	    printf("file not found \n");
	    exit(0);
    }
    read_lines_and_render(fp);
}

void read_stdin()
{
    FILE *fp = fdopen(STDIN_FILENO, "r");
    if (fp == NULL)
    {
	    printf("stdin not found \n");
	    exit(0);
    }
    read_lines_and_render(fp);
}


int main(int argc, char *argv[])
{
    SHOW_LOG = 0;

    prefix = NULL;
    suffix = NULL;
    printRaw = false;

    bool is_text = false;
    bool is_stdin_disabled = isatty(0);

#ifdef CHECK_UPDATE
    if (argc == 1)
    {
        check_version();
        printf("sequence-diagram-cli %s\ngithub@Ja-sonYun, email: jason@abex.dev\ngithub: https://github.com/Ja-sonYun/sequence-diagram-cli\nwebsite: https://abex.dev\n", VERSION);

        return 0;
    }

    printf("*** sequence-diagram-cli %s, github@Ja-sonYun ***\n", VERSION);
#endif

    int option_starting_pos = is_stdin_disabled ? STANDARD_OPTION_POSITION : STDIN_OPTION_POSITION;
    for (int i = option_starting_pos; i < argc; i++)
    {
        if (!strncmp(argv[i], "log=", 4))
        {
            if (strlen(argv[i]) == 5 && (argv[i][4] == 1 || argv[i][4 == 0]))
            {
                SHOW_LOG = argv[i][4] - 48;
            }
            else
            {
                printf(KRED"wrong log option!\n"RESET);
            }
        }
        else if (!strcmp(argv[i], "al"))
        {
            is_text = true;
        }
        else if (!strcmp(argv[i], "raw"))
        {
            printRaw = true;
        }
        else if (!strncmp(argv[i], "prefix=", 7))
        {
            if (strlen(argv[i]) > 7)
            {
                prefix = &argv[i][7];
            }
            else
            {
                printf(KRED"wrong prefix option!\n"RESET);
            }
        }
        else if (!strncmp(argv[i], "suffix=", 7))
        {
            if (strlen(argv[i]) > 7)
            {
                suffix = &argv[i][7];
            }
            else
            {
                printf(KRED"wrong suffix option!\n"RESET);
            }
        }
        else
        {
            printf(KRED"wrong option!\n"RESET);
        }
    }

    set_style(is_text);

    bool file_exists = access(argv[1], F_OK) == 0;
    if (!file_exists && is_stdin_disabled)
    {
        printf("file not found \n");
    }
    else if (file_exists && !is_stdin_disabled)
    {
        printf("Undefined behaviour: stdin and file specified\n");
    }
    else if (is_stdin_disabled)
    {
        readfile(argv[1]);
    }
    else
    {
        read_stdin();
    }


#ifdef CHECK_UPDATE
    check_version();
#endif

    return 0;
}

