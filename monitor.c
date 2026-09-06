#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "monitor.h"
#include "log.h"
#include "config.h"

#define MAX_PROC 10000

static int throttled[MAX_PROC] = {0};
static int original_priority[MAX_PROC] = {0};

/* Read total CPU time from /proc/stat */
void read_total_cpu_time(long long *total)
{
    FILE *f = fopen("/proc/stat", "r");

    if (!f)
        return;

    char line[256];

    fgets(line, sizeof(line), f);
    fclose(f);

    long long user, nice, system, idle;
    long long iowait, irq, softirq, steal;

    sscanf(line,
           "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
           &user,
           &nice,
           &system,
           &idle,
           &iowait,
           &irq,
           &softirq,
           &steal);

    *total = user + nice + system + idle +
             iowait + irq + softirq + steal;
}

/* Read process CPU time from /proc/[pid]/stat */
void read_process_cpu_time(int pid, long long *time)
{
    char path[64];

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *f = fopen(path, "r");

    if (!f)
        return;

    char buffer[1024];

    fgets(buffer, sizeof(buffer), f);
    fclose(f);

    long long utime, stime;

    sscanf(buffer,
           "%*d %*s %*c %*d %*d %*d %*d %*d "
           "%*u %*u %*u %*u %*u "
           "%lld %lld",
           &utime,
           &stime);

    *time = utime + stime;
}

/* Get number of CPU cores */
static int get_num_cores()
{
    FILE *f = fopen("/proc/cpuinfo", "r");

    if (!f)
        return 1;

    int cores = 0;
    char line[256];

    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "processor", 9) == 0)
            cores++;
    }

    fclose(f);

    return cores > 0 ? cores : 1;
}

/* Scan and monitor processes */
void scan_processes()
{
    int cores = get_num_cores();

    DIR *dir;
    struct dirent *entry;

    long long total1, total2;

    static long long prev_proc[MAX_PROC] = {0};

    /* FIRST PASS */

    read_total_cpu_time(&total1);

    dir = opendir("/proc");

    if (!dir)
        return;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!isdigit(entry->d_name[0]))
            continue;

        int pid = atoi(entry->d_name);

        if (pid >= MAX_PROC)
            continue;

        read_process_cpu_time(pid, &prev_proc[pid]);
    }

    closedir(dir);

    /* Sampling interval */
    sleep(1);

    /* SECOND PASS */

    read_total_cpu_time(&total2);

    long long total_diff = total2 - total1;

    dir = opendir("/proc");

    if (!dir)
        return;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!isdigit(entry->d_name[0]))
            continue;

        int pid = atoi(entry->d_name);

        if (pid >= MAX_PROC)
            continue;

        /* Ignore system processes and daemon itself */
        if (pid < 100 || pid == getpid())
            continue;

        long long current;

        read_process_cpu_time(pid, &current);

        if (total_diff <= 0)
            continue;

        long long proc_diff = current - prev_proc[pid];

        double cpu =
            (double)proc_diff /
            (double)total_diff *
            100.0 *
            cores;

        /* High CPU */
        if (cpu > THRESHOLD_HIGH)
        {
            char msg[128];

            snprintf(msg,
                     sizeof(msg),
                     "High CPU: PID %d -> %.2f%%",
                     pid,
                     cpu);

            write_log(msg);

            throttle_process(pid);
        }

        /* Low CPU */
        else if (cpu < THRESHOLD_LOW)
        {
            restore_process(pid);
        }
    }

    closedir(dir);
}

/* Increase nice value to reduce CPU priority */
void throttle_process(int pid)
{
    if (pid >= MAX_PROC || throttled[pid])
        return;

    int pr = getpriority(PRIO_PROCESS, pid);

    original_priority[pid] = pr;

    int new_pr = pr + 5;

    if (new_pr > 19)
        new_pr = 19;

    if (setpriority(PRIO_PROCESS, pid, new_pr) == 0)
    {
        throttled[pid] = 1;

        char msg[128];

        snprintf(msg,
                 sizeof(msg),
                 "Throttled PID %d (nice %d -> %d)",
                 pid,
                 pr,
                 new_pr);

        write_log(msg);
    }
}

/* Restore original nice value */
void restore_process(int pid)
{
    if (pid >= MAX_PROC || !throttled[pid])
        return;

    if (setpriority(PRIO_PROCESS,
                     pid,
                     original_priority[pid]) == 0)
    {
        throttled[pid] = 0;

        char msg[128];

        snprintf(msg,
                 sizeof(msg),
                 "Restored PID %d (nice -> %d)",
                 pid,
                 original_priority[pid]);

        write_log(msg);
    }
}
