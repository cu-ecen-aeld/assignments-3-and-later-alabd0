/**
 * @file aesdsocket.c
 * @author Abdullah Alabd (abdullah.alabd.else@gmail.com)
 * @brief Assignment 5 - Part - 1 Socket Programming (ADVANCED EMBEDDED LINUX COURSE)
 * @version 0.1
 * @date 2023-08-31
 *
 * @copyright Copyright (c) 2023
 *
 */

/* _________________ INCLUDES________________________ */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "queue.h"
#include <pthread.h>
#include <time.h>
#include <sys/time.h>


/* _______________ MACROS DEFINITIONS ________________________ */




#define AESD_CHAR_DRIVE_USING                                   /*  REMOVE/ADD  COMMENT FOR USING/UNUSING TIME AESD CHAR DRIVE  */
// #define USE_TIME_STAMP                                       /*  REMOVE/ADD  COMMENT FOR USING/UNUSING TIME STAMP ___ */
#ifdef AESD_CHAR_DRIVE_USING
#define OPENFILEFLAGS (O_RDWR | O_SYNC)
#else
#define OPENFILEFLAGS (O_RDWR | O_CREAT | O_TRUNC | O_SYNC)
#endif
#define LISTENER_ADDRESS ((const char *)NULL)
#define LISTENER_PORT ((const char *)"9000")
#define MAX_CONNECTION_REQUESTS ((int)20)
#define MAX_OPENED_FD ((int)20)
#define RETURN_OK ((int)0)
#define RETURN_NOT_OK ((int)-1)
#ifdef AESD_CHAR_DRIVE_USING
    #define RECEIVED_DATA_FILE_LOCATION ((const char *)"/dev/aesdchar")
#else
    #define RECEIVED_DATA_FILE_LOCATION ((const char *)"/var/tmp/aesdsocketdata")
#endif

/* _______________ VARS DECs/DEFS ________________________ */

int err;                              // SAVE errno VAR
struct addrinfo hints, *res, *p;      // SERVER ADDRESSES GENERATOR
struct sockaddr_storage clientaddr;   // RETURN OF ACCEPTED CLIENT ADDRESS
int allfd_count;                      // STORE THE COUNT OF ANY OPENED FD
int allfd[MAX_OPENED_FD];             // STORE ANY OPENED FD
char buffer_client_addresss[50];      // RETURN HUMAN RIDEABLE ADDRESS.
int lisnerfd = -1, fdwriterfile = -1 , fdwriterfile_timer = -1; // THE FD OF LISTENER & FD OF FILE TRACKING FOR ALL RECEIVED DATA.
ssize_t sendbuffer_size;              // @sendbuffer ELEMENT SIZE FOR REALLOCATING
char *sendbufptr;                     // KEEPING TRACKING OF LAST READ ELEMENT IN @sendbuffer
ssize_t timer_len_buffer = 0;              
pthread_mutex_t the_mutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct slist_data_s slist_data_t;
struct slist_data_s {
    char flag;
    pthread_t thread;
    SLIST_ENTRY(slist_data_s) entries;
};
typedef struct{
    int fd;
    slist_data_t * datap;
}pthread_arg_struct_t;


/*______________ FUNCTION DECs ___________________ */

/**
 * @brief loop in all opened fd and close it.
 *
 */
static void closs_all_fd(void);

/**
 * @brief 
 * 
 * @param arg 
 * @return void* 
 */
void *start_thread(void *arg);


/**
 * @brief Identify the binary IP address if it v4 Or v6
 *
 * @param a Structure Pointer describing a generic socket address
 * @return void* Return a POINTER TO sockaddr_in6 OR sockaddr_in
 */
static void *getaddinetntop(struct sockaddr *a);

/**
 * @brief Send the receive to specific fd client
 *
 * @param fds fd client
 * @param buf the buffer containing the received data send back to client.
 * @param len the lens of buffer and the remaining lens of
 * @return @param RETURN_NOT_OK if it fails
 *         @param RETURN_OK if it OK.
 */
//static int sendall(int fds, char *buf, int *len);
/**
 * @brief signals initializing set the signals handlers
 *
 * @return @param RETURN_NOT_OK if it fails
 *         @param RETURN_OK if it OK.
 *
 */
int signal_initializing(void);

int timer_reader_initializing(void);

/**
 * @brief initializing the server .. bind and get ready to listen to requests.
 *
 * @return @param RETURN_NOT_OK if it fails
 *         @param RETURN_OK if it OK.
 */
static int listener_socket_initializing(void);

/**
 * @brief accept request, receive the data, write to a file @param RECEIVED_DATA_FILE_LOCATION
 *        the read the file and send it back to the client.
 *
 * @return int
 */
static int super_loop_accept_receive_write_sendback(void);

/**
 * @brief Gracefully exits when SIGINT or
 *        SIGTERM is received , Closing all FDs ,
 *        Deallocating Memory, Terminating
 *        andy waiting zombies.
 *
 * @param signo SIGNAL
 *
 *  */

void sigint_handler(int signo)
{   
#ifdef USE_TIME_STAMP
    if (signo == SIGALRM)
    {
        char outstr[80] = {0};
        time_t t;
        struct tm *tmp;
        t = time(NULL);
        tmp = localtime(&t);
        if (tmp == NULL)
        {
            perror("localtime");
            exit(EXIT_FAILURE);
        }
        if (strftime(outstr, 80, "timestamp: %F %T\n", tmp) == 0)
        {
            fprintf(stderr, "strftime returned 0");
            exit(EXIT_FAILURE);
        }
            //int pos;
            int pos=lseek (fdwriterfile, 0, SEEK_CUR);
        if (lseek(fdwriterfile, (off_t)timer_len_buffer, SEEK_END) == -1)
        {
            err = errno;
            fprintf(stderr, "lseek Read ERROR FUNCTION: %s\n", strerror(err));
            exit( RETURN_NOT_OK);
        }
        
        write(fdwriterfile, (void *)outstr, strlen(outstr));
        write(STDOUT_FILENO, (void *)outstr, strlen(outstr));


        if (timer_len_buffer != 0){
            if (lseek(fdwriterfile, (off_t)pos, SEEK_SET) == -1)
            {
                err = errno;
                fprintf(stderr, "lseek Read ERROR FUNCTION: %s\n", strerror(err));
                exit( RETURN_NOT_OK);
            }
        }
    }
    else
#endif
    if ((signo == SIGTERM) || (signo == SIGINT))
    {
        write(STDOUT_FILENO, (void *)"Caught signal, exiting!\n", 24);
        closs_all_fd();
        syslog(LOG_MAKEPRI(LOG_USER, LOG_INFO), "Caught signal, exiting!");
        closelog();
#ifndef AESD_CHAR_DRIVE_USING
        system("rm -rf /var/tmp/aesdsocketdata");
#endif
        fflush(stdout);
        exit(RETURN_OK);
    }
    else if (signo == SIGCHLD)
    {
        while (waitpid(-1, NULL, WNOHANG) > 0)
            ; // Caught SIGCHLD, KILL ZOMBIES!

    }
}

int main(int argc, char *argv[])
{
    if (signal_initializing() == RETURN_NOT_OK)
    {
        fprintf(stderr, "signel_initializing FUNCTION ERROR\n");
        return RETURN_NOT_OK;
    }

    if ((argc > 2) || (argc == 2 && (strcmp(argv[1], "-d") != 0)))
    {
        fprintf(stderr, "ERROR ARGUMENTS ARE NOT SUPPORTED!\n");
        return RETURN_NOT_OK;
    }

    if (listener_socket_initializing() == RETURN_NOT_OK)
    {
        fprintf(stderr, "listener_socket_initializing FUNCTION ERROR\n");
        return RETURN_NOT_OK;
    }

    if ((fdwriterfile = open(RECEIVED_DATA_FILE_LOCATION, OPENFILEFLAGS , 0664)) == -1)
    {
        err = errno;
        fprintf(stderr, "open ERROR FUNCTION: %s\n", strerror(err));
        return RETURN_NOT_OK;
    }
    allfd[++allfd_count] = fdwriterfile;

    /* Put the program in the background, and dissociate from the controlling
   terminal.  If NOCHDIR is zero, do `chdir ("/")'.  If NOCLOSE is zero,
   redirects stdin, stdout, and stderr to /dev/null.  */

    if (argc > 1 && !(strcmp(argv[1], "-d")))
    {
        if (RETURN_NOT_OK == daemon(0, 1))
        {
            err = errno;
            fprintf(stderr, "daemon ERROR: %s\n", strerror(err));
            return RETURN_NOT_OK;
        }
    }
#ifdef USE_TIME_STAMP
    if (timer_reader_initializing() == RETURN_NOT_OK)
    {
        fprintf(stderr, "timer_reader_initializing FUNCTION ERROR\n");
        return RETURN_NOT_OK;
    }
#endif

    if (super_loop_accept_receive_write_sendback() == RETURN_NOT_OK)
    {
        fprintf(stderr, "super_loop_accept_recieve_write_sendback FUNCTION ERROR\n");
        return RETURN_NOT_OK;
    }

    return RETURN_OK;
}

static void *getaddinetntop(struct sockaddr *a)
{
    if (a->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)a)->sin_addr);
    }
    else
        return &(((struct sockaddr_in6 *)a)->sin6_addr);
}

static void closs_all_fd(void)
{
    for (; allfd_count != 0; allfd_count--)
    {
        close(allfd[allfd_count]);
    }
}

int signal_initializing(void)
{
    struct sigaction sact;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;

    sact.sa_handler = &sigint_handler;

    if (sigaction(SIGINT, &sact, NULL) == RETURN_NOT_OK)
    {
        err = errno;
        fprintf(stderr, "signal ERROR FUNCTION: %s\n", strerror(err));
        return RETURN_NOT_OK;
    }

    if (sigaction(SIGTERM, &sact, NULL) == RETURN_NOT_OK)
    {
        err = errno;
        fprintf(stderr, "signal ERROR FUNCTION: %s\n", strerror(err));
        return RETURN_NOT_OK;
    }
    if (sigaction(SIGCHLD, &sact, NULL) == RETURN_NOT_OK)
    {
        err = errno;
        fprintf(stderr, "signal ERROR FUNCTION: %s\n", strerror(err));
        return RETURN_NOT_OK;
    }
#ifdef USE_TIME_STAMP
    if (sigaction(SIGALRM, &sact, NULL) == RETURN_NOT_OK)
    {
        err = errno;
        fprintf(stderr, "signal ERROR FUNCTION: %s\n", strerror(err));
        return RETURN_NOT_OK;
    }
#endif


    return RETURN_OK;
}
#ifdef USE_TIME_STAMP
int timer_reader_initializing(void)
{
    struct itimerval delay;
    delay.it_value.tv_sec = 10;
    delay.it_value.tv_usec = 0;
    delay.it_interval.tv_sec = 10;
    delay.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &delay, NULL))
    {
        err = errno;
        fprintf(stderr, "setitimer ERROR FUNCTION: %s\n", strerror(err));
        return RETURN_NOT_OK;
    }
    return RETURN_OK;
}
#endif

static int listener_socket_initializing(void)
{
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(LISTENER_ADDRESS, LISTENER_PORT, &hints, &res))
    {
        err = errno;
        fprintf(stderr, "getaddrinfo ERROR FUNCTION: %s\n", strerror(err));
        return RETURN_NOT_OK;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((lisnerfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {

            continue;
        }
        break;
    }

    if (lisnerfd == -1)
    {
        fprintf(stderr, "socket ERROR FUNCTION: %s\n", strerror(err));
        return RETURN_NOT_OK;
    }
    allfd[++allfd_count] = lisnerfd;

    int yes = 1;

    if (setsockopt(lisnerfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        err = errno;
        fprintf(stderr, "setsockopt ERROR FUNCTION: %s\n", strerror(err));
        close(lisnerfd);
        return RETURN_NOT_OK;
    }

    if (bind(lisnerfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        err = errno;
        fprintf(stderr, "bind ERROR FUNCTION: %s\n", strerror(err));
        close(lisnerfd);
        return RETURN_NOT_OK;
    }

    if (listen(lisnerfd, MAX_CONNECTION_REQUESTS) == RETURN_NOT_OK)
    {
        err = errno;
        fprintf(stderr, "listen ERROR FUNCTION: %s\n", strerror(err));
        close(lisnerfd);
        return RETURN_NOT_OK;
    }

    freeaddrinfo(res);

    return RETURN_OK;
}

static int super_loop_accept_receive_write_sendback(void)
{  
    slist_data_t *datap = NULL , * np_temp=NULL;
    SLIST_HEAD(slisthead, slist_data_s) head;
    SLIST_INIT(&head);
    pthread_t thread;
    int fdclient = RETURN_NOT_OK;

    socklen_t clientaddr_len = sizeof(clientaddr);

    // pthread routine... 
    while (1)
    {
        // pthread accept connection ... 

        while ((fdclient = accept(lisnerfd, (struct sockaddr *)&clientaddr, &clientaddr_len)) == RETURN_NOT_OK)
        {
            err = errno;
            fprintf(stderr, "accept ERROR FUNCTION: %s\n", strerror(err));
        }
        allfd[++allfd_count] = fdclient; 
        inet_ntop(clientaddr.ss_family, getaddinetntop((struct sockaddr *)&clientaddr), buffer_client_addresss, 50);
        syslog(LOG_MAKEPRI(LOG_USER, LOG_INFO), "Accepted connection from %s\n", buffer_client_addresss);
        memset(buffer_client_addresss, 0, 50);

        
        datap = (slist_data_t *)malloc(sizeof(slist_data_t));
        pthread_arg_struct_t *pthread_arg_arr = (pthread_arg_struct_t *)malloc(sizeof(pthread_arg_struct_t));
        pthread_arg_arr->datap = datap;
        pthread_arg_arr->fd = fdclient;


        if (pthread_create (&thread, NULL, start_thread, (void *)pthread_arg_arr) != 0 ) {
            err = errno;
            fprintf(stderr, "pthread_create FUNCTION: %s\n", strerror(err));
            return RETURN_NOT_OK;
        }

        datap->thread = thread;
        datap->flag = 0;
        SLIST_INSERT_HEAD(&head, datap, entries);
        

        // Read & Remove;
        SLIST_FOREACH_SAFE(datap, &head, entries, np_temp)
        {
            if (datap->flag == 1)
            {
                pthread_join(datap->thread, NULL);
                SLIST_REMOVE(&head, datap, slist_data_s, entries);
                free(datap);
            }
        }
        
    }
    return RETURN_NOT_OK;
}

void *start_thread(void *arg)
{
    pthread_arg_struct_t * argv = (pthread_arg_struct_t *)(arg);
    int fdclient = argv->fd;
    char rcvbuf[100000];
    ssize_t len_buffer = 0;
    ssize_t ret = 0;
    int numbytes = 0;

    if ((numbytes = recv(fdclient, rcvbuf, 100000, 0)) == -1)
    {
        err = errno;
        fprintf(stderr, "recv ERROR FUNCTION: %s\n", strerror(err));
        exit(RETURN_NOT_OK);
    }
    rcvbuf[numbytes] = '\n';

    char *rcvbufptr = rcvbuf;
    len_buffer = numbytes;
    pthread_mutex_lock (&the_mutex);
    timer_len_buffer = len_buffer;
    while (timer_len_buffer != 0 && (ret = write(fdwriterfile, rcvbufptr, timer_len_buffer)) != 0)
    {
        if (ret == -1)
        {
            if (errno == EINTR)
                continue;
            err = errno;
            fprintf(stderr, "write ERROR FUNCTION: %s\n", strerror(err));
            exit(RETURN_NOT_OK);
            break;
        }
        timer_len_buffer -= ret;
        
        rcvbufptr += ret;
    }
#ifndef AESD_CHAR_DRIVE_USING
    if (lseek(fdwriterfile, (off_t)0, SEEK_SET) == -1)
    {
        err = errno;
        fprintf(stderr, "lseek Read ERROR FUNCTION: %s\n", strerror(err));
        exit( RETURN_NOT_OK);
    }
    #endif
char buf[500000] = {0};
start:
    ret = 1;
    int buf_offset = 0;
    while (ret)
    {   
        ret = read(fdwriterfile, buf + buf_offset, 500000);
        if (ret == RETURN_NOT_OK)
        {   
            err = errno;
            if (errno == EINTR)
                goto start; /* oh shush */
            else
            {
                fprintf(stderr, "Read sendbuffer ERROR FUNCTION: %s\n", strerror(err));
                exit( RETURN_NOT_OK);
            }
        }
        else{
            buf_offset += ret;
        }
    }

    fprintf(stdout, "contents of the buf: %s\n", buf);
    
    
    int bytesleft = strlen(buf);
    send(fdclient, buf, bytesleft, 0);
#ifndef AESD_CHAR_DRIVE_USING
    if (lseek(fdwriterfile, (off_t)0, SEEK_END) == -1)
    {
        err = errno;
        fprintf(stderr, "lseek Read ERROR FUNCTION: %s\n", strerror(err));
        exit( RETURN_NOT_OK);
    }
#endif

    close(fdclient);
    argv->datap->flag = 1;
    free(argv);
    pthread_mutex_unlock (&the_mutex);
    return (void *)NULL;
}