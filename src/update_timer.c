#include <syslog.h>
#include <stdio.h>
#include <time.h>


void update_timer(struct tm *due_date) {
    printf("Current day is %d; changing timer to tomorrow's day\n", due_date->tm_mday);
    due_date->tm_mday += 1;

    // Converts the struct tm to a time_t
    mktime(due_date);  
    syslog(LOG_INFO, "Timer updated, due next day: %d", due_date->tm_mday);
}
