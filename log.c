#include <stdio.h>
#include <time.h>

#include "log.h"
#include "config.h"

void write_log(const char *message)
{
    char path[600];

    snprintf(path,
             sizeof(path),
             "%s/log.txt",
             PROJECT_DIR);

    FILE *f = fopen(path, "a");

    if (!f)
        return;

    time_t now = time(NULL);

    struct tm *t = localtime(&now);

    fprintf(f,
            "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec,
            message);

    fclose(f);
}
