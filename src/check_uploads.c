#include <unistd.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

void check_uploads(void)
{
    openlog("check_uploads", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "check_uploads() called");

    const char *filenames[] = {
        "/home/bango/Documents/SystemSoftwareAssignment/upload/Warehouse.json",
        "/home/bango/Documents/SystemSoftwareAssignment/upload/Manufacturing.json",
        "/home/bango/Documents/SystemSoftwareAssignment/upload/Sales.json",
        "/home/bango/Documents/SystemSoftwareAssignment/upload/Distribution.json"};

    const char *message_to_pipe;

    // Create the named pipe if it doesn't exist
    if (mkfifo("/tmp/report_fifo", 0666) == -1)
    {
        if (errno != EEXIST)
        {
            syslog(LOG_ERR, "Could not create named pipe: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    syslog(LOG_INFO, "Named pipe created or already exists");

    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0)
    {
        syslog(LOG_ERR, "fork failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        syslog(LOG_INFO, "fork() successful, child PID: %d", pid);
    }

    if (pid == 0)
    {
        // Child process
        syslog(LOG_INFO, "Child process started");
        FILE *output_file = fopen("/home/bango/Documents/SystemSoftwareAssignment/upload/uploads.txt", "a");
        if (output_file == NULL)
        {
            syslog(LOG_ERR, "Failed to open the file for writing: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        syslog(LOG_INFO, "File opened for writing");

        int all_files_exist = 1; // Assume all files exist initially

        syslog(LOG_INFO, "Starting file checking");
        for (int i = 0; i < (int)(sizeof(filenames) / sizeof(filenames[0])); i++)
        {
            struct stat file_info;

            if (stat(filenames[i], &file_info) == 0 && S_ISREG(file_info.st_mode))
            {
                fprintf(output_file, "File: %s\n", filenames[i]);
                fprintf(output_file, "Owner: %s\n", getpwuid(file_info.st_uid)->pw_name);
                fprintf(output_file, "Last Modified: %s", ctime(&file_info.st_mtime));
                fprintf(output_file, "Last Accessed: %s", ctime(&file_info.st_atime));
                fprintf(output_file, "Recorded at: %s", ctime(&(time_t){time(NULL)})); // Current date and time
                fprintf(output_file, "\n");
            }
            else
            {
                fprintf(output_file, "File missing : %s\n", filenames[i]);
                all_files_exist = 0; // If any file is missing, set all_files_exist to 0
            }
        }
        syslog(LOG_INFO, "File checking completed");

        fclose(output_file);

        // Open the named pipe in write mode
        int pipe_fd = open("/tmp/report_fifo", O_WRONLY);
        if (pipe_fd == -1)
        {
            syslog(LOG_ERR, "Failed to open the named pipe for writing: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        syslog(LOG_INFO, "Named pipe opened for writing");

        // Write a message to the pipe indicating whether the file checking was successful or not
        if (all_files_exist)
        {
            message_to_pipe = "File checking has been successful";
        }
        else
        {
            message_to_pipe = "File checking has failed";
        }
        write(pipe_fd, message_to_pipe, strlen(message_to_pipe));

        close(pipe_fd);
        syslog(LOG_INFO, "Message written to pipe and pipe closed");
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Parent process
        syslog(LOG_INFO, "Parent process started");
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            syslog(LOG_INFO, "Child process exited, status: %d", WEXITSTATUS(status));
            // Open the named pipe in read mode
            int pipe_fd = open("/tmp/report_fifo", O_RDONLY);
            if (pipe_fd == -1)
            {
                syslog(LOG_ERR, "Failed to open the named pipe for reading: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            syslog(LOG_INFO, "Named pipe opened for reading");

            char buffer[128];
            ssize_t num_read = read(pipe_fd, buffer, sizeof(buffer) - 1);
            if (num_read == -1)
            {
                syslog(LOG_ERR, "Failed to read from the named pipe: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }

            buffer[num_read] = '\0'; // Null-terminate the string
            syslog(LOG_INFO, "Received message: %s", buffer);
            syslog(LOG_INFO, "Read %zd bytes from pipe", num_read);

            close(pipe_fd);
            syslog(LOG_INFO, "Message read from pipe and pipe closed");
        }
    }
    closelog();
}