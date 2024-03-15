#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void backup_dash(void)
{
    openlog("backup_dash", LOG_PID, LOG_USER);

    const char *backup_dir = "/home/bango/Documents/SystemSoftwareAssignment/backup";
    const char *dashboard_dir = "/home/bango/Documents/SystemSoftwareAssignment/dashboard";

    // Fork the process to create the backup directory
    pid_t pid = fork();
    if (pid == -1)
    {
        syslog(LOG_ERR, "Failed to fork: %m");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Child process: Create the backup directory
        if (execlp("mkdir", "mkdir", "-p", backup_dir, NULL) == -1)
        {
            syslog(LOG_ERR, "Failed to create backup directory: %m");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Parent process: Wait for the child to complete
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            syslog(LOG_INFO, "Backup directory created successfully");
        }
        else
        {
            syslog(LOG_ERR, "Failed to create backup directory");
            exit(EXIT_FAILURE);
        }
    }

    // Fork again to copy files from dashboard to backup
    pid = fork();
    if (pid == -1)
    {
        syslog(LOG_ERR, "Failed to fork: %m");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Child process: Copy files from dashboard to backup
        if (execlp("cp", "cp", "-r", dashboard_dir, backup_dir, NULL) == -1)
        {
            syslog(LOG_ERR, "Failed to copy files to backup: %m");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Parent process: Wait for the child to complete
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            syslog(LOG_INFO, "Files copied to backup successfully");
        }
        else
        {
            syslog(LOG_ERR, "Failed to copy files to backup");
            exit(EXIT_FAILURE);
        }
    }

    closelog();
}
