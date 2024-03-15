#ifndef DAEMON_TASKS 
#define DAEMON_TASKS

#include <time.h>

void collect_reports(void);

void backup_dash(void);

void lock_directories(void);

void unlock_directories(void );

void generate_reports(void);

void check_uploads(void);

void sig_handler(int);

void update_timer(struct tm*);
#endif
