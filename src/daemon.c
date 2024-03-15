#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h> 
#include "daemon_task.h"

int main()
{
   time_t now;
   struct tm backup_time;
   struct tm check_uploads_time;

   // Initialize current, backup, and upload check times
   time(&now);
   struct tm *current_time = localtime(&now);

   // Set backup time to 01:00 of the current or next day
   backup_time = *current_time;
   backup_time.tm_hour = 01;
   backup_time.tm_min = 00;
   backup_time.tm_sec = 0;
   if (difftime(now, mktime(&backup_time)) > 0) backup_time.tm_mday++;

   // Set upload check time to 23:30 of the current or next day
   check_uploads_time = *current_time;
   check_uploads_time.tm_hour = 23;
   check_uploads_time.tm_min = 30;
   check_uploads_time.tm_sec = 0;
   if (difftime(now, mktime(&check_uploads_time)) > 0) check_uploads_time.tm_mday++;

   // Start daemon
   openlog("daemon", LOG_PID, LOG_DAEMON);
   syslog(LOG_INFO, "Starting daemon");

   // Create child process
   int pid = fork();
   if (pid > 0)
   {
      // Parent process exits
      syslog(LOG_INFO, "Exiting parent process");
      exit(EXIT_SUCCESS);
   }
   else if (pid == 0)
   {
      // Create orphan process and elevate to session leader
      if (setsid() < 0)
      {
         syslog(LOG_ERR, "Error creating new session: %s", strerror(errno));
         exit(EXIT_FAILURE);
      }

      pid = fork();
      if (pid > 0)
      {
         exit(EXIT_SUCCESS);
      }
      else
      {
         syslog(LOG_INFO, "Running child process");

         // Set file mode creation mask to 0 and change working dir to root
         umask(0);
         if (chdir("/") < 0)
         {
            syslog(LOG_ERR, "Error changing directory to root");
            exit(EXIT_FAILURE);
         }

         // Close all open file descriptors
         for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) close(x);

         syslog(LOG_INFO, "Daemon started");

         // Main loop
         while (1)
         {
            sleep(1);

            // Handle SIGINT signal
            if (signal(SIGINT, sig_handler) == SIG_ERR)
            {
               syslog(LOG_ERR, "ERROR: daemon.c : SIG_ERR RECEIVED");
            }

            // Check for XML uploads
            time(&now);
            double seconds_to_files_check = difftime(mktime(&check_uploads_time), now);
            syslog(LOG_INFO, "%.f seconds until check for xml uploads", seconds_to_files_check);
            if (seconds_to_files_check <= 0)
            {
               check_uploads();
               update_timer(&check_uploads_time);
            }

            // Perform backup and transfer, countdown to 23:30
            time(&now);
            double seconds_to_transfer = difftime(mktime(&backup_time), now);
            syslog(LOG_INFO, "%.f seconds until backup & transfer", seconds_to_transfer);
            if (seconds_to_transfer <= 0)
            {
               lock_directories();
               collect_reports();
               backup_dash();
               sleep(30);
               unlock_directories();
               update_timer(&backup_time);
            }
         }
      }
      closelog();
      return 0;
   }
}