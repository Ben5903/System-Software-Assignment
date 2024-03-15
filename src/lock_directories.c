#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <string.h>

void lock_directories() {
    openlog("lock_directories", LOG_PID, LOG_USER);

    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %m");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        if (execl("/bin/chmod", "chmod", "g-rwx", "/home/bango/Documents/SystemSoftwareAssignment/upload", NULL) == -1) {
            syslog(LOG_ERR, "execl failed: %m");
            exit(1);
        }
    } else {
        // Parent process
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) == 0) {
            syslog(LOG_INFO, "Directory 'Upload locked");
        } else {
            syslog(LOG_ERR, "Error locking directory 'Upload'");
            exit(EXIT_FAILURE);
        }
    }

    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %m");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        if (execl("/bin/chmod", "chmod", "g-rwx", "/home/bango/Documents/SystemSoftwareAssignment/dashboard", NULL) == -1) {
            syslog(LOG_ERR, "execl failed: %m");
            exit(1);
        }
    } else {
        // Parent process
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                syslog(LOG_INFO, "Directory 'dashboard' locked");
            } else {
                syslog(LOG_ERR, "Error locking directory 'dashboard'");
                exit(EXIT_FAILURE);
            }
        }
    }
    closelog();
}