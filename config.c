#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

double THRESHOLD_HIGH = 10;
double THRESHOLD_LOW = 2;
int SLEEP_TIME = 2;

char PROJECT_DIR[512] = "";

void load_config()
{
    char path[600];

    snprintf(path,
             sizeof(path),
             "%s/config.txt",
             PROJECT_DIR);

    FILE *f = fopen(path, "r");

    if (!f)
        return;

    char line[128];

    while (fgets(line, sizeof(line), f))
    {
        /* Remove newline */
        line[strcspn(line, "\r\n")] = '\0';

        if (strncmp(line, "THRESHOLD_HIGH=", 15) == 0)
        {
            THRESHOLD_HIGH = atof(line + 15);
        }

        else if (strncmp(line, "THRESHOLD_LOW=", 14) == 0)
        {
            THRESHOLD_LOW = atof(line + 14);
        }

        else if (strncmp(line, "SLEEP_TIME=", 11) == 0)
        {
            SLEEP_TIME = atoi(line + 11);
        }
    }

    fclose(f);
}
