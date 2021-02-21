#include "main.h"

void readfile(char *filename)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("file not found!\n");
        exit(0);
    }

    int CUR_LINE_NUM = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        CUR_LINE_NUM++;
        if (strcmp(line, "\n") && line[0] != ';')
        {
            if (SHOW_LOG > 1)
            {
                int line_s = strlen(line) + 3;
                char sep[line_s];
                memset(sep, '-', line_s);
                sep[line_s] = '\0';
                printf(KGRN"%s\n", sep);
                printf(KBLU" +"KCYN" %s\n"RESET, line);
            }
            line[strlen(line)-1] = '\0';
            parse_line(line, CUR_LINE_NUM);
        }
    }

    render();

    fclose(fp);
    if (line)
        free(line);
}

int main(int argc, char *argv[])
{
    SHOW_LOG = 0;

    prefix = NULL;
    suffix = NULL;

    bool is_text = false;

    if (argc == 1)
    {
        check_version();
        printf("sequence-diagram-cli %s\ngithub@Ja-sonYun, email: jason@abex.dev\ngithub: https://github.com/Ja-sonYun/sequence-diagram-cli\nwebsite: https://abex.dev\n", VERSION);

        return 0;
    }

    printf("*** sequence-diagram-cli %s, github@Ja-sonYun ***\n", VERSION);
    for (int i = 2; i < argc; i++)
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

    readfile(argv[1]);


    check_version();

    return 0;
}

