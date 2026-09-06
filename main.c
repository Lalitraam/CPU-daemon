#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "monitor.h"
#include "config.h"

/* Capture the real project directory before daemonization */
static void capture_project_dir()
{
    if (getcwd(PROJECT_DIR, sizeof(PROJECT_DIR)) == NULL)
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

static void daemonize()
{
    /* Check if daemon is already running */
    FILE *f = fopen("/tmp/cpu_daemon.pid", "r");

    if (f)
    {
        int old_pid;
        fscanf(f, "%d", &old_pid);
        fclose(f);

        if (kill(old_pid, 0) == 0)
        {
            printf("Daemon already running with PID %d\n", old_pid);
            exit(EXIT_FAILURE);
        }
    }

    /* First fork */
    pid_t pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Create new session */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Second fork */
    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Reset file permissions */
    umask(0);

    /* Stay in project directory */
    if (chdir(PROJECT_DIR) != 0)
        exit(EXIT_FAILURE);

    /* Redirect standard file descriptors */
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

    /* Write PID file */
    FILE *fp = fopen("/tmp/cpu_daemon.pid", "w");

    if (fp)
    {
        fprintf(fp, "%d\n", getpid());
        fclose(fp);
    }
}

int main()
{
    capture_project_dir();

    load_config();

    daemonize();

    while (1)
    {
        load_config();

        scan_processes();

        sleep(SLEEP_TIME);
    }

    return 0;
}
