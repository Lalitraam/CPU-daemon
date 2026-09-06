#ifndef CONFIG_H
#define CONFIG_H

extern double THRESHOLD_HIGH;
extern double THRESHOLD_LOW;
extern int SLEEP_TIME;
extern char PROJECT_DIR[512];

void load_config(void);

#endif
