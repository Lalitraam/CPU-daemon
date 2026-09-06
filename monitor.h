#ifndef MONITOR_H
#define MONITOR_H

void read_total_cpu_time(long long *total);
void read_process_cpu_time(int pid, long long *time);
void scan_processes(void);
void throttle_process(int pid);
void restore_process(int pid);

#endif
