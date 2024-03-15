#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h> 

void collect_reports(void) {
    openlog("collect_reports", LOG_PID, LOG_USER);

    pid_t pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "fork failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        syslog(LOG_INFO, "About to execute mv command");
        char *envp[] = { NULL }; // Declare envp variable
        if (execle("/bin/sh", "sh", "-c", "mv /home/bango/Documents/SystemSoftwareAssignment/upload/*.json /home/bango/Documents/SystemSoftwareAssignment/report", NULL, envp) == -1)  {
            syslog(LOG_ERR, "execl failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        syslog(LOG_INFO, "mv command executed successfully");
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            syslog(LOG_INFO, "Reports collected");
        } else {
            syslog(LOG_ERR, "Reports not collected");
            syslog(LOG_ERR, "Child process exited with status: %d", WEXITSTATUS(status));
            exit(EXIT_FAILURE);
        }
    }

    closelog();
}