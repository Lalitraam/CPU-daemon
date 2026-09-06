# CPU-daemon
A lightweight Linux background daemon written in C that monitors real-time CPU usage of running processes and automatically throttles processes that exceed a defined CPU threshold — restoring their priority once usage drops back down.

Features
  Runs as a proper Unix daemon (double-fork, detached from terminal, PID-file locked to prevent duplicate instances)
  Calculates per-process CPU usage by sampling /proc/[pid]/stat and /proc/stat over an interval
  Automatically adjusts process priority (nice value) via setpriority() when CPU usage exceeds THRESHOLD_HIGH
  Restores original priority once usage drops below THRESHOLD_LOW
  Reloads configuration (config.txt) on every monitoring cycle — no restart needed to change thresholds
  Logs all throttle/restore events with timestamps to log.txt

Tech Stack
  C (POSIX APIs: unistd.h, signal.h, sys/resource.h)
  Linux /proc filesystem for process and CPU stats
  Built with make

Build & Run
  make
  ./cpu_daemon


How It Works

The daemon samples each process's CPU time twice, one second apart, and compares the delta against total system CPU time (scaled by core count) to compute a percentage. Processes crossing THRESHOLD_HIGH get their nice value increased (lower priority); once they drop below THRESHOLD_LOW, their original priority is restored.
