#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define FAILURE_PROGRAM ((int)1)
#define SUCCESSFULL_PROGRAM ((int)0)

int writer_status = FAILURE_PROGRAM;
struct tm* ptr_time;
time_t t_time_now;
void find_time(void);

int main (int argc, char *argv[])
{   
    printf("Program name %s\n", argv[0]);
    int fd;
    errno = 0;
    int err = errno; 
    if (argc == 3)
    {   
        fd = open (argv[1], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0664);
        if (fd == -1)
        {
            err = errno; 
            fprintf(stdout,"Error: %s\n", strerror(err));
            find_time();
            syslog(LOG_MAKEPRI(LOG_USER, LOG_ERR), "[%s] %02i:%02i:%02i :\neError Message: %m\n", "LOG_ERROR",ptr_time->tm_hour,ptr_time->tm_min,ptr_time->tm_sec);
            writer_status = FAILURE_PROGRAM;
        }
        else if ( write(fd,argv[2], strlen(argv[2]) ) == -1)
        {   
            err = errno; 
            fprintf(stdout,"Error: %s", strerror(err));
            find_time();
            syslog(LOG_MAKEPRI(LOG_USER, LOG_ERR), "[%s] %02i:%02i:%02i :\neError Message: %m\n", "LOG_ERROR",ptr_time->tm_hour,ptr_time->tm_min,ptr_time->tm_sec);
        }
        else {
            find_time();
            syslog(LOG_MAKEPRI(LOG_USER, LOG_DEBUG), "[%s] %02i:%02i:%02i :\nWriting %s to %s\n", "LOG_DEBUG",ptr_time->tm_hour,ptr_time->tm_min,ptr_time->tm_sec,argv[2],argv[1]);
            writer_status = SUCCESSFULL_PROGRAM;
        }
        
        
    }
    else {
        fprintf(stderr,"Error In The Arguments\n");
        find_time();
        syslog(LOG_MAKEPRI(LOG_USER, LOG_ERR), "[%s] %02i:%02i:%02i :\nERROR: %s\n", "LOG_ERROR",ptr_time->tm_hour,ptr_time->tm_min,ptr_time->tm_sec,"Error In Arguments");   

    }
    closelog();
    if (fd != -1) close(fd);
    exit(writer_status);
}

void find_time(void)
{
    time(&t_time_now);
    ptr_time = localtime(&t_time_now);
}