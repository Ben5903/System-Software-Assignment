#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h> 

void unlock_directories() {
    openlog("unlock_directories", LOG_PID, LOG_USER);
    printf("Unlocking directories\n");

    const char *directories[] = {"/home/bango/Documents/SystemSoftwareAssignment/upload", "/home/bango/Documents/SystemSoftwareAssignment/upload", "/home/bango/Documents/SystemSoftwareAssignment/dashboard"};

    for (int i = 0; i < (int)(sizeof(directories) / sizeof(directories[0])); i++) {
        pid_t pid = fork();
        if (pid < 0) {
            syslog(LOG_ERR, "fork failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Child process
            if (execl("/bin/chmod", "chmod", "g+rwx", directories[i], NULL) == -1) {
                syslog(LOG_ERR, "execl failed: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                syslog(LOG_INFO, "Directory '%s' unlocked", directories[i]);
            } else {
                syslog(LOG_ERR, "Error unlocking directory '%s'", directories[i]);
                exit(EXIT_FAILURE);
            }
        }
    }

    closelog();
}
